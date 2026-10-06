# How do you build the rules into a device?

## Introduction.

The other notes of this section say what the rules ask. This one says how you answer, on a real board, with real option names.

Is that a lot of new engineering? Less than it sounds. Open the datasheet of the microcontroller you already use and search for three things: a boot ROM that checks signatures, flash encryption, and a fuse for the debug port. Most likely all three are there, and most likely your product ships with all three switched off. So this note is mostly about switches: which ones exist, what each one costs, and in which order to flip them before a unit leaves the factory.

It goes one requirement at a time. You get the mechanism in plain words first. After that comes a table that shows it on the three families that turn up most in IoT designs, which are the **ESP32** with ESP-IDF, the **STM32**, and the **Nordic nRF** with the nRF Connect SDK. Some tables have a fourth row for an answer that does not depend on the chip.

!!! note "Versions"
    Option names are from **ESP-IDF v6.1**, **nRF Connect SDK v3.4**,
    **Zephyr v4.4**, **MCUboot v2.4** and the STM32Cube packages, read in
    October 2026. They change
    between versions, and not every chip of a family has every feature. Treat
    the names as search terms for the documentation of the version you use.
    And one warning that applies to half of this page: most of these
    mechanisms burn one-time memory. Try them on boards you can afford to
    lose.

## The map.

| The rules ask for | Who asks | The standard mechanism |
|---|---|---|
| only your firmware runs | RED, CRA | secure boot |
| vulnerabilities can be fixed in the field | RED, CRA | signed updates, two slots, a way back |
| secrets stay secret | RED, CRA | encrypted flash, keys that hardware can use and nobody can read |
| every device is its own identity | RED, CRA | a key pair and a certificate per device |
| no default password | RED, CRA | a secret per device, or a setup that forces one |
| protected communication | RED, CRA | TLS, authenticated pairing |
| a small attack surface | CRA | debug ports locked, unused services off |
| one bug does limited damage | CRA | stack protection, memory protection, isolation |
| data can be erased for good | CRA | a factory reset that destroys keys |
| you know what is inside | CRA | an SBOM from the build, checked against vulnerability lists |
| radio power inside the limits | RED, FCC | a certified module, power tables, a locked region |
| limited time on air | RED, FCC | an airtime budget in the stack |
| the device survives the lab | RED | filters, ESD protection, a watchdog |

## Secure boot: only your firmware runs.

**The mechanism.** Ask a simple question: when the chip wakes up, who decides what code runs? With secure boot the answer is a small piece of code that was fixed at the factory, a ROM or a bootloader whose flash is locked for good. It keeps the hash of your public key in one-time memory, and it refuses to start anything that is not signed with the matching private key. The stage it starts repeats the check on the application, so the chain reaches all the way up. Notice that the device only ever holds the public half. The private key is in your hands, and [How do signatures and certificates prove who you are?](../cybersecurity/how_do_signatures_prove_identity.md) explains why a public key is all the device needs.

| Platform | How it is done |
|---|---|
| ESP32 | **Secure Boot V2** (`CONFIG_SECURE_BOOT`). The ROM verifies the bootloader, and the bootloader verifies the application at boot and on every update. Most chips use RSA-3072, and some use ECDSA. The key digest lives in eFuse. Most chips hold up to three digests and can revoke one. |
| STM32 | It depends on the series. On the STM32H5, the parts with hardware crypto have **STiRoT**, a root of trust that ST puts in the chip, and the others use **OEMiRoT**, which is built on MCUboot. STM32U5, STM32L5 and STM32WBA have MCUboot based boot with TF-M. Older series use the X-CUBE-SBSFU package. ST still maintains it and no longer develops it, and points new designs to the MCUboot based options. |
| Nordic nRF | Two stages. The **nRF Secure Immutable Bootloader** (`SB_CONFIG_SECURE_BOOT_APPCORE`) is locked and verifies **MCUboot** (`SB_CONFIG_BOOTLOADER_MCUBOOT`), which can itself be updated. Key hashes sit in one-time memory on the nRF5340 and the nRF91, and in the key unit of the nRF54L. Signatures are ECDSA P-256, or Ed25519 on the nRF54L. |
| Any MCU | **MCUboot** in a flash region that you write-protect. It verifies RSA, ECDSA P-256 or Ed25519 signatures. Some chips bring their own: the RP2350 boot ROM checks a signature against key hashes in its one-time memory. |

Three things go wrong here, and none of them is technical.

- **You lose the private key.** Then no update can ever be signed again. Keep it in a hardware security module, with a backup in a second place.
- **You have one key and it leaks.** Many chips can store several key digests, and that feature is there for this day. Burn two or three at the factory and lock the spare private keys away. A leaked key can only be revoked if its replacement is already inside every device.
- **You enable it too late.** Secure boot burns fuses on the first boot. A production line that flashes first and thinks later produces bricks.

## Updates: two slots, a signature and a way back.

**The mechanism.** Give the application two homes in the flash, and only ever write to the one that is not running. When a new image has been downloaded there, the bootloader verifies its signature and then lets it boot **on trial**. What happens next depends on the image. An image that starts, does its checks and says "I am fine" is promoted and stays. An image that crashes, hangs or stays silent is thrown out at the next reset, and the old one comes back. One more guard closes the last hole: a counter in one-time memory, so that nobody can install last year's firmware with last year's vulnerability.

| Platform | How it is done |
|---|---|
| ESP32 | Partitions `ota_0`, `ota_1` and `otadata`, and the `esp_https_ota` component for the download. Rollback is `CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE`: the new image starts as pending, and the application calls `esp_ota_mark_app_valid_cancel_rollback()` when it is satisfied. Anti-rollback is `CONFIG_BOOTLOADER_APP_ANTI_ROLLBACK`, with a version number stored in eFuse. |
| Nordic nRF and Zephyr | MCUboot either swaps the two slots, which keeps the old image for a revert, or overwrites the old one. The application confirms with `boot_write_img_confirmed()`. Downgrade protection comes in two kinds: by version number (`CONFIG_MCUBOOT_DOWNGRADE_PREVENTION`, in overwrite mode) or by a hardware counter (`CONFIG_MCUBOOT_HW_DOWNGRADE_PREVENTION`). The image arrives over Bluetooth LE (`CONFIG_MCUMGR_TRANSPORT_BT`), over the cellular modem with nRF Cloud, or over your own transport. |
| STM32 | The same idea in each secure boot package: two slots, a signature check, and a version check against a counter that cannot go back. On the MCUboot based ones it is the flow of the row above, and the new image reverts unless the application confirms it. |
| Small flash | When two slots do not fit inside the chip, the second slot goes to an external SPI flash, and MCUboot can keep that copy encrypted. |

Two details save you later.

The anti-rollback counter is **finite**. Take the ESP32. Its counter is a row of fuse bits, and every step burns one of them. The default is 16 steps. The classic ESP32 has 32, the ESP32-C5 has 9, and the little ESP32-C2 has 4. So do not raise the secure version on every build. Raise it when a release closes a vulnerability that you never want to see again.

The confirmation is where you decide what "it works" means. Confirm after the device has reached its server with the new firmware, and not at the first line of `main`. A firmware that boots and cannot connect is a firmware that can never be replaced.

## Secrets and identity: keys that nobody can read.

**The mechanism.** Think of an attacker with a hot-air station who takes the flash chip off your board and reads it. Two layers stand in the way. The outer layer is **encryption** of the whole flash, with a key that is burned into fuses and that the hardware uses without ever showing it, so the dump is noise. The inner layer is for the one secret that matters most, the long-term private key of the device. That key goes somewhere firmware can say "sign this" and get a signature back, but can never say "show me the key". Depending on the chip, that somewhere is a dedicated peripheral, a secure world behind TrustZone, or a small separate chip called a secure element.

Once the key has a safe home, giving the device an identity is the usual three steps. At the factory the device makes its own key pair. It sends out a CSR. The factory CA answers with a certificate. If any of those words is new, the cybersecurity notes of this site go through them one by one.

| Platform | Encrypted storage | A key that cannot be read |
|---|---|---|
| ESP32 | Flash encryption (`CONFIG_SECURE_FLASH_ENC_ENABLED`), with the key in eFuse. NVS encryption (`CONFIG_NVS_ENCRYPTION`) for the key-value store. | The **Digital Signature** peripheral signs with an RSA key that is stored encrypted and that only the hardware can decrypt. Newer chips also have an ECDSA peripheral with the key in eFuse. The `esp_secure_cert_mgr` component keeps the certificate next to it. |
| STM32 | Readout protection keeps the internal flash from being read out. The series with a hardware unique key, such as STM32U5 and STM32H5, can also wrap keys with it, so that they can be used and cannot be read. | TF-M secure storage on the TrustZone series, or the STSAFE-A secure element from ST with its STSELib library. |
| Nordic nRF | The trusted storage library (`CONFIG_TRUSTED_STORAGE`), encrypted with a key derived from the hardware unique key. | The **Key Management Unit** of the nRF5340, the nRF91 and the nRF54L: firmware pushes a key into the crypto engine and never sees it. On the nRF91, the modem generates and keeps its own credentials. |
| Any MCU | Whatever the chip offers. | A **secure element** on I2C: Microchip ATECC608, NXP EdgeLock SE05x, Infineon OPTIGA Trust M. The private key is born inside it and never leaves. |

Small teams should look twice at the secure element, for a reason that has nothing to do with cryptography. Microchip, NXP and Infineon all sell versions that come **already provisioned**. You open the reel and each chip already holds a key pair and a certificate signed by its maker. The hardest part of a secure factory, which is running a certificate authority on a production line, is done before the parts reach you.

One trap on the ESP32: flash encryption has a development mode and a release mode. Development mode leaves the serial bootloader able to write new firmware. It exists for your desk. A device that ships in development mode is not protected, and changing the option in the menu does not fix a unit that already booted. For that there is a function, `esp_flash_encryption_set_release_mode()`.

## No default password.

**The mechanism.** The security standard of the RED puts it in one requirement: a factory password is unique for each unit and strong, or the user is forced to change it at first use. The same standard asks that guessing be made slow. There are four patterns that everybody uses, and you will recognise them.

| Pattern | How it works | Where you have seen it |
|---|---|---|
| a secret on the label | each unit carries its own code, printed as text or as a QR code, and the setup needs it | Matter: a passcode of 27 bits and a discriminator of 12 bits inside the QR code |
| a public key on the label | the QR code carries the public key of the device, and the phone uses it to send the network credentials encrypted | Wi-Fi Easy Connect |
| a code on a screen | the device shows six digits and the user types or compares them | Bluetooth LE pairing with passkey or numeric comparison |
| forced setup | the device refuses to work until the user has chosen a password | many home routers |

ESP-IDF ships the first pattern ready to use, in its provisioning component. It has three security schemes and only one is worth shipping: scheme 2, built on SRP6a (`CONFIG_ESP_PROTOCOMM_SUPPORT_SECURITY_VERSION_2`). Scheme 0 sends everything unprotected and is there for experiments. Where does the secret of each unit come from? From a small factory partition. A tool called `mfg_gen.py` takes a spreadsheet with one row per device and produces one partition image for each.

On Bluetooth LE, whatever the chip, the choice that matters is the pairing method. "Just Works" pairing has no protection against somebody in the middle. Passkey entry and numeric comparison do. What if the device has no screen and no keys? It can still pair properly: print the code on its label, or pass it with an NFC tap.

One reminder from the RED note belongs here. If the setup lets the user skip the password, the product loses the easy route to the CE mark. Leave that button out.

## Protected communication.

**The mechanism.** For anything that crosses the internet, it is TLS, with three conditions that people forget. The device **checks the certificate of the server** against a list of roots that it carries. The device **has a clock that is roughly right**, since every certificate has a start date and an end date. And when the server has to know which device is on the line, the device shows a certificate of its own and proves it with the key from the previous section. The handshake itself is in [How does TLS work?](../cybersecurity/how_does_tls_work.md).

| Platform | How it is done |
|---|---|
| ESP32 | The `esp-tls` component over Mbed TLS. The root list is one line, `.crt_bundle_attach = esp_crt_bundle_attach`, and it carries the Mozilla roots. TLS 1.3 is `CONFIG_MBEDTLS_SSL_PROTO_TLS1_3`. Time comes from SNTP. |
| Nordic nRF | On the nRF91, TLS runs inside the modem with credentials that you store by number and never read back. On the others, Mbed TLS runs on top of the PSA crypto API, so the keys can stay in the key unit. |
| STM32 | Mbed TLS or wolfSSL as middleware, with the private key operations sent to TF-M or to a secure element. |

Radio links that do not use TLS have their own version of the same idea. Bluetooth LE has authenticated pairing, as above. LoRaWAN gives every device a root key, and if there is one secret on the board that deserves a secure element, it is that one.

## Debug ports and other open doors.

**The mechanism.** A debug port reads and writes everything. In production it has to be closed, or it has to ask who is there. The same goes for the serial bootloader in the ROM, and for any console that your firmware leaves on a UART.

| Platform | How it is closed |
|---|---|
| ESP32 | Enabling secure boot or flash encryption burns the JTAG fuses (`DIS_PAD_JTAG`, `DIS_USB_JTAG`) by itself, unless you ask it not to. The serial bootloader is one option, `CONFIG_SECURE_UART_ROM_DL_MODE`: a restricted "secure download mode", or disabled completely. |
| STM32 | Readout protection level 2 switches the debug port off. On the classic series that is for ever. On the STM32U5 and its relatives it can be undone only with a key that you stored before. The STM32H5 replaces the levels with product states: **Closed** shuts the port and can be reopened with **debug authentication**, by password or by certificate, and **Locked** is final. |
| Nordic nRF | Access port protection: `CONFIG_NRF_APPROTECT_LOCK`, and `CONFIG_NRF_SECURE_APPROTECT_LOCK` on the TrustZone parts. Unlocking means a full erase of the chip, so the firmware and its secrets go with it. |
| RP2350 | A fuse, `CRIT1.DEBUG_DISABLE`. |

Two warnings from the documentation, both easy to miss.

On the nRF54L, the protection is **off by default** in the SDK, which is convenient on your desk. Production builds have to set the lock option explicitly.

On the ESP32, there is a middle road for field returns. A "soft" disable of JTAG can be undone by firmware that knows an HMAC key. It gives your repair team a way in that a stranger does not have. It needs planning, because the default fuses of secure boot close JTAG the hard way.

The rest of the attack surface is not a fuse. It is a list: the shell that somebody left for testing, the Bluetooth service that nobody uses, the web server on a forgotten port. Read your own firmware for them before the lab does.

## When a bug gets through.

**The mechanism.** Every firmware has bugs, and the CRA asks you to limit what one bug can do. There are three layers, from cheap to expensive.

| Layer | ESP32 | Zephyr and nRF |
|---|---|---|
| detect a smashed stack | `CONFIG_COMPILER_STACK_CHECK_MODE`, set to strong | `CONFIG_STACK_CANARIES` |
| stop code from running where data lives | `CONFIG_ESP_SYSTEM_MEMPROT`, on by default | `CONFIG_HW_STACK_PROTECTION`, and `CONFIG_USERSPACE` with the memory protection unit |
| put the keys behind a wall | ESP-TEE on the ESP32-C6, ESP32-H2 and ESP32-C5 | TF-M on the TrustZone parts: nRF5340, nRF91, and also STM32U5, STM32L5 and STM32H5 |

Zephyr adds a small tool that is worth one minute of every release: `west build -t hardenconfig`. What you get back is the list of options where your build is weaker than the hardened settings that the project recommends.

A watchdog belongs in this section too. It does not stop an attack. It makes sure that a device knocked into a strange state comes back by itself, and that is what the availability requirement is about.

## Erasing for real, and remembering what happened.

**The mechanism.** Somebody sells your sensor second-hand. Can the buyer dig the Wi-Fi password of the first owner out of the flash? After a factory reset the answer has to be no, and erasing sectors does not guarantee it: it is slow, and wear levelling can leave old copies in blocks that you cannot see. The trick that works is older than flash memory. Keep the data of the user encrypted from the first day, and at reset time destroy **the key**. Without the key, what remains is noise.

Be careful with what you erase. The Wi-Fi password and the account token belong to the user and must go. The device certificate belongs to the device and must stay, or the unit can never be set up again.

The security log is the other half. Nobody is asking for a database. A ring of a few hundred entries in flash will do, each with a timestamp and one line. The events worth a line are few: a failed login, an update whose signature did not verify, a change of configuration, a factory reset. The CRA asks that the user be able to switch it off.

## Knowing what is inside: the SBOM.

**The mechanism.** The list of software components is generated **by the build**, on every release, in one of the two standard formats: SPDX or CycloneDX. A second tool compares that list with the public vulnerability databases, every night, for every release that is still in the field.

| Platform | The command |
|---|---|
| ESP32 | `esp-idf-sbom create build/project_description.json` writes the list, in SPDX or CycloneDX. `esp-idf-sbom check` compares it with the NVD database. |
| Zephyr and nRF | `west spdx --init -d build`, then build with `CONFIG_BUILD_OUTPUT_META` on, then `west spdx -d build`. It writes SPDX files for the application, for Zephyr and for the modules. |
| STM32 | Every STM32Cube package ships a CycloneDX file, `sbom_cdx.json`, for the code that ST wrote. The list for your whole build comes from a general tool. |
| Any project | Syft or cdxgen to generate, and OSV-Scanner, Grype or Dependency-Track to watch. |

A German guideline, BSI TR-03183-2, is the most precise description of what a good SBOM contains. It is a useful checklist until the official CRA standards are published.

The contact address is one small file. Create a text file called `security.txt` and serve it at `/.well-known/security.txt` on your website, because that is where a researcher will look first. It must contain a `Contact` line and an `Expires` line. Everything else in it is optional.

The last piece is knowing your fleet. Picture the morning a vulnerability is announced. How many of your devices run the affected version? You want to answer that before the coffee is cold, and you can only do it if each device reports its firmware version every time it connects and you store what it says. ESP RainMaker, nRF Cloud and Memfault are services that keep that record for you. A column in your own database is just as good.

## Radio: power, region and time on air.

**The mechanism.** Start with a **certified module** and use it the way it was certified. That one decision moves most of the radio work to somebody who has already done it. What is left for your firmware is three things: the power, the region and the time on air.

| Family | Modules that carry their own certifications |
|---|---|
| ESP32 | Espressif's own modules, such as the ESP32-C3-MINI-1 and the ESP32-S3-WROOM-1. The certificates are on its website. |
| Nordic nRF | u-blox NORA-B1, Raytac MDBT53 and Ezurio BL54L15, all built around Nordic chips. |
| STM32 | STM32WB5MMG and STM32WBA5MMG from ST for Bluetooth. For LoRa, the RAK3172 and the Seeed Wio-E5, both built around the STM32WL. |

Be careful with the part number when you order. Under one module name there are usually several variants, one with a chip antenna, one with a trace antenna, one with a connector, and each variant got its own certificate. The certificate you need is the one that matches your bill of materials to the last letter.

**Power and region.** No radio stack can obey a limit that it does not know about. So it needs two facts from you: the country it is operating in, and the highest power it may use there.

| Platform | How it is done |
|---|---|
| ESP32 | `esp_wifi_set_country_code()` sets the country. The default is a "world safe" setting. `esp_wifi_set_max_tx_power()` caps the power, in steps of 0.25 dBm. For limits per country, the PHY data can be switched by region (`CONFIG_ESP_PHY_MULTIPLE_INIT_DATA_BIN`), with separate entries for FCC, CE and others. Bluetooth has its own call, `esp_ble_tx_power_set()`. |
| Nordic nRF | Bluetooth power is a build option, `CONFIG_BT_CTLR_TX_PWR_PLUS_8` and its siblings. With an external amplifier, the gain is `CONFIG_MPSL_FEM_NRF21540_TX_GAIN_DB`, and it counts towards the limit. The nRF70 Wi-Fi chip takes a country in `CONFIG_NRF70_REG_DOMAIN`. |
| STM32 | On the Bluetooth series the call is `aci_hal_set_tx_power_level()`. On the STM32WL, the LoRa series, the region is chosen in the LoRaWAN stack. Since STM32CubeWL v1.6 that stack is Semtech's LoRa Basics Modem, and the call is `smtc_modem_set_region()`. |

On every platform the same rule from the other notes applies. Region and power table are factory data, written once on the production line, and you will not find them in a settings menu.

**Time on air.** If you use LoRaWAN, the stack already keeps the accounts. Semtech's **LoRa Basics Modem**, the one it recommends for new designs, has a duty cycle module with a sliding window of one hour. The older LoRaMac-node does the same with time credits per sub-band. In the United States there is nothing to switch on. The stack simply refuses payloads that would stay on a channel for more than 0.4 seconds, and at the slowest data rate that leaves room for 11 bytes.

With plain LoRa and no LoRaWAN, nobody keeps the accounts for you. Compute the time on air of each packet and keep a budget per sub-band over the last hour.

```
Tsym      = 2^SF / BW
Tpreamble = (Npreamble + 4.25) × Tsym
Npayload  = 8 + max( ceil( (8·PL − 4·SF + 28 + 16·CRC − 20·IH) / (4·(SF − 2·DE)) ) × (CR + 4), 0 )
ToA       = Tpreamble + Npayload × Tsym
```

`PL` is the payload in bytes, `CRC` is 1 when the CRC is on, `IH` is 1 when there is no header, `DE` is 1 when the low data rate option is on, and `CR` is 1 for a coding rate of 4/5. A packet of 23 bytes at SF12 and 125 kHz gives 1483 ms. At a duty cycle of 1 %, the device then has to stay quiet for about 147 seconds.

Some countries ask the radio to listen before it talks, Japan and South Korea among them. That check is a measurement of energy on the channel. The channel activity detection of LoRa chips is a different thing and does not replace it.

## The hardware that the lab looks for.

Some answers are not in the firmware at all.

| What the lab measures | The usual hardware |
|---|---|
| harmonics of the transmitter | a matching network after the amplifier that also works as a low-pass filter, and a shield over the radio |
| receiver blocking | a band-pass filter, often a SAW filter, in front of the receiver |
| static discharge: 4 kV by contact, 8 kV through the air | protection diodes on every connector and button, as close to the connector as they fit |
| fast transients and voltage dips | a brown-out detector that resets cleanly, and a watchdog |

The immunity tests have a rule that firmware people like, once they know it. During a discharge the device is allowed to glitch. What it must do is come back by itself, without losing important data and without transmitting by accident. That is a firmware requirement in disguise: a clean reset path, a watchdog that is really armed, and settings that are written to flash in a way that survives a reset in the middle.

A certified module helps here as well. The matching network, the filter and the shield are inside it, and they were in place when it was measured.

## The order at the factory.

Fuses do not have an undo. Get the order wrong on the production line and no setting will save the unit. The sequence below is a reasonable starting point.

1. Long before production, generate the signing keys, and keep them somewhere a stolen laptop cannot expose.
2. Program the bootloader and the firmware, and make sure both are the signed builds.
3. Ask the device to create its key pair, send its CSR to the factory CA, and store the certificate that comes back.
4. Write what makes this unit different from the next one: serial number, setup secret, region.
5. Enable flash encryption and secure boot. The first boot burns the fuses.
6. Close the debug port and the serial bootloader.
7. Read back what the chip still allows, compare it with what you intended, and add the unit to your database.

Before the real batch, spend a handful of boards on a rehearsal of the whole sequence. Those boards are the cheapest insurance in the project, because a mistake in step 5 cannot be reflashed away.

## Easy to get wrong.

- **"Secure boot protects my secrets."** It checks who wrote the firmware. It does not encrypt anything. You need flash encryption as well.
- **"Flash encryption stops modified firmware."** It hides the contents. It does not check a signature. You need secure boot as well.
- **"Development mode is good enough to ship."** On the ESP32 it leaves a way to reflash the chip over the serial port.
- **"The debug port is closed by default."** On some chips it is open until your build closes it. Check the default of your exact part.
- **"I can roll the version counter as often as I like."** It is a row of fuses, and some chips have four.
- **"The SBOM is a document that I write once."** It is an output of the build, and it changes with every release.
- **"A certified module settles the radio."** Only with its antenna, its power settings and its distance to the body.

## Related.

- [What rules does an IoT device have to pass before you can sell it?](what_rules_does_an_iot_device_have_to_pass.md) — the map of the rules themselves
- [What does the RED ask of an IoT device?](what_does_the_red_ask_of_an_iot_device.md) — the European radio and security requirements
- [What does the CRA ask of an IoT device?](what_does_the_cra_ask_of_an_iot_device.md) — the list that most of this note answers
- [What does the FCC ask of an IoT device?](what_does_the_fcc_ask_of_an_iot_device.md) — the American radio rules
- [How do signatures and certificates prove who you are?](../cybersecurity/how_do_signatures_prove_identity.md) — the idea behind secure boot and device identity

!!! note "Reference"
    The vendor documentation is the place to check every option name:
    the [ESP-IDF security guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c6/security/security.html),
    the [nRF Connect SDK security pages](https://nrfconnectdocs.nordicsemi.com/ncs/latest/nrf/security.html),
    the [MCUboot documentation](https://docs.mcuboot.com/) and
    [Trusted Firmware-M](https://www.trustedfirmware.org/projects/tf-m/).
    The security standard of the RED, EN 18031-1, can be read for free, and
    its requirements on passwords and updates are short. The LoRa time on air
    formula is from the Semtech SX1276 datasheet.
