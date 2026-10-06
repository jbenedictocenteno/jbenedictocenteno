# What does the FCC ask of an IoT device?

## Introduction.

In the United States, the radio spectrum has a landlord, and its name is the FCC. Its main worry is easy to state. Some radio services paid a lot of money for their frequencies, others keep aeroplanes and ambulances safe, and your device must not get in their way. Almost every rule you will meet comes out of that single worry.

An IoT device lives in the part of the rules called **Part 15**, inside Title 47 of the Code of Federal Regulations. Part 15 is the deal for radios that need no licence. You can transmit without asking anybody, and in exchange you accept two conditions. Your device may not cause harmful interference. And it must put up with any interference it receives.

There is one big difference from Europe, and it is worth knowing from the first line. In the EU you test the product and declare by yourself that it complies. In the United States a transmitter cannot be sold until somebody else has reviewed it and given it a number, the **FCC ID**.

The device of this note is the same one as in the other notes of this section. It is a small battery sensor with a Wi-Fi and Bluetooth LE module at 2.4 GHz. Sometimes it also carries a LoRa radio at 915 MHz, and that second radio makes things more interesting.

!!! note "Versions"
    This note describes the rules as they stand in **October 2026**. The radio
    limits have been stable for years. The rules about who made your parts are
    changing every few months, and one of them takes effect on 13 October 2026.
    I am an engineer, not a lawyer. Use the note as a map, and check the
    [current text of Part 15](https://www.ecfr.gov/current/title-47/part-15)
    and your test lab before you decide anything that costs money.

## Two kinds of emission, two procedures.

Part 15 splits every product into two halves, and each half has its own paperwork.

The first half is the **unintentional radiator**. That is the digital part of the product: the processor, the clocks, the switching regulator. It does not mean to emit radio, and it emits a little anyway. A device for homes is "Class B" and gets the stricter limits. A device for offices and factories is "Class A". For this half you can sign a **Supplier's Declaration of Conformity**: you test it, you keep the report, and a party based in the United States takes responsibility.

The second half is the **intentional radiator**, which is the transmitter. There is no self-declaration here. You need **certification**: an accredited lab measures the device, and a private body called a TCB reviews the report and issues the grant, with the FCC ID, in the name of the FCC.

The sensor is both things at once. So the usual product has a certified transmitter and a declaration for everything else.

## What each band allows.

Each band has its own section of Part 15, and the limits are very different from one band to the next.

| Band | Used by | Section | Main limits |
|---|---|---|---|
| 902 to 928 MHz | LoRa, other sub-GHz radios | 15.247 | 1 W conducted, with conditions on bandwidth or hopping |
| 2400 to 2483.5 MHz | Wi-Fi, Bluetooth, Zigbee, Thread | 15.247 | 1 W conducted; 8 dBm in any 3 kHz |
| 5.15 to 5.85 GHz | Wi-Fi | 15.407 | 250 mW for a client in most of it; radar detection (DFS) in the middle sub-bands |
| 5.925 to 7.125 GHz | Wi-Fi 6E and 7 | 15.407 | 24 dBm e.i.r.p. for a client of an indoor access point; 14 dBm for very low power devices |
| 13.56 MHz | NFC | 15.225 | 15,848 µV/m measured at 30 m |
| 125 kHz | RFID | 15.209 | 19.2 µV/m measured at 300 m |
| 315 and 433 MHz | remote controls | 15.231 | short control signals only, not a data stream |

Three things in this table need a comment.

**The power is conducted power.** The FCC limit is what the transmitter delivers to the antenna connector. On top of it you may use an antenna with up to 6 dBi of gain. With more gain than that, you lower the power by the same number of decibels. Europe does it the other way: its limit is what leaves the antenna.

**1 W is a lot.** At 2.4 GHz the European limit is 100 mW radiated. The FCC allows ten times more into the antenna. A sensor never gets near that, so the power limit is almost never the thing that fails the test. The next section explains what does.

**There is a low-power way in.** Section 15.249 allows a transmitter in these bands with almost no conditions, if its field stays under 50 mV/m at 3 m. That is a bit less than 1 mW radiated. It is enough for a short link, and it skips the rules about bandwidth and hopping.

## What really stops you: the restricted bands.

Part 15 has a list of **restricted bands** in section 15.205. They belong to aviation, satellites, radio astronomy and similar services. In those bands your device may only emit the tiny level that any electronic device is allowed to leak. Above 960 MHz that level is 500 µV/m at 3 m. In power, that is about −41 dBm radiated, which is more than ten million times less than the 1 W of the previous table.

Two of those bands sit exactly where an IoT radio hurts.

**The band edge at 2483.5 MHz.** Where the 2.4 GHz band stops, at 2483.5 MHz, a restricted band begins, with no gap between them. Now think about Bluetooth LE: its last channel is centred on 2480 MHz, which leaves 3.5 MHz of room. Wi-Fi is a little better off on channel 11, the highest one used in the United States, but not by much. No signal ends in a clean wall, so the skirt of yours spills over the edge, and whatever spills has to be under −41 dBm. This is the most common failure of a 2.4 GHz product at the lab.

Most products get out of this with a **power table per channel**. In the middle of the band the firmware uses full power, and on the last channels it gives up a few decibels. A cleaner amplifier or a filter also helps, but the table is cheap and it is what most products do.

**The harmonics.** A transmitter also emits weak copies of its signal at two, three and more times its frequency. Look where they fall:

| Radio | Harmonic | Falls at | Restricted band there |
|---|---|---|---|
| 915 MHz | third | 2.7 to 2.8 GHz | 2690 to 2900 MHz |
| 915 MHz | fourth | 3.6 to 3.7 GHz | 3600 to 4400 MHz |
| 915 MHz | fifth | 4.5 to 4.6 GHz | 4.5 to 5.15 GHz |
| 2.4 GHz | second | 4.8 to 5.0 GHz | 4.5 to 5.15 GHz |

Harmonics do not care about your firmware. They are fixed on the board, with three old remedies: a low-pass filter after the amplifier, a metal shield over the radio, and the reference layout of the chip copied without improvements. This is one of the best reasons to buy a certified module, since the filter, the shield and the layout come inside it.

## LoRa at 915 MHz: hop, or be wide.

Section 15.247 offers two ways to transmit up to 1 W, and a narrow LoRa signal fits neither of them easily.

The first way is **digital modulation**. For that, the signal must be 500 kHz wide or more, and it may not concentrate more than 8 dBm in any slice of 3 kHz. A LoRa signal of 500 kHz qualifies. A LoRa signal of 125 kHz does not, because it is too narrow.

The second way is **frequency hopping**. A narrow signal is allowed if it keeps moving. For a signal narrower than 250 kHz, moving means 50 channels at the very least, and no more than 0.4 seconds on any one of them in every 20 seconds.

Anyone who has configured LoRaWAN for the United States has seen these numbers without knowing where they came from. It has 64 uplink channels of 125 kHz to hop over, and 8 channels of 500 kHz that count as digital modulation. And those 0.4 seconds are the reason the slowest data rate there carries only a few bytes per message: a longer message would stay on the channel for too long.

Notice what is missing. In Europe the sub-GHz band limits your **duty cycle**, often to 1 % of the time. The FCC has no duty cycle at 915 MHz. It has the dwell time per channel. The same firmware cannot be right in both places. Whatever is correct for Europe breaks the American rule, and the other way round, so the region has to be decided in the build. It is not a setting to leave in the hands of a user.

## The antenna is part of the grant.

Section 15.203 says that the antenna has to be permanently attached, or it has to use a connector that is not a standard one. The idea is that nobody should be able to screw a bigger antenna onto your product.

The grant lists the antennas that were tested. After that:

- the same type of antenna with the same gain or less is fine, with no new test;
- a different type, or more gain, needs a new filing with new measurements.

So do not treat the antenna as a late decision. A chip antenna, a PCB trace and a wire are three different types. If you use a module that was certified with a trace antenna, you copy that trace exactly, with the same board thickness.

## Modules: borrowing somebody else's grant.

Certifying a transmitter from zero is rare in IoT. The normal path is a **module** that somebody already took through certification, with its own FCC ID, and Part 15 lets your product ride on that grant. If you remember one shortcut from this note, make it this one.

A module does not get that kind of grant easily. The FCC wants a radio that would behave the same in any product, so the module brings everything with it: its shield, its voltage regulation, buffered data inputs, and an antenna that respects the antenna rule. It is tested alone, outside any product.

When you put that module in your sensor, three duties stay with you.

1. **The label.** The outside of the product says `Contains FCC ID:` followed by the ID of the module.
2. **The digital half.** The module grant covers the transmitter only. Your whole product still has to pass the unintentional radiator limits, with the module inside and working.
3. **The conditions of the grant.** Every module comes with integration instructions. They fix the antenna, and they usually fix a minimum distance to the body.

That third point breaks more products than people expect. Three changes take you outside the grant of the module:

| You do this | Why it matters | What it costs |
|---|---|---|
| use a different type of antenna, or more gain | the grant covers the tested antennas only | new measurements and a filing on the module grant |
| put the device closer to the body than the grant says | most modules are approved for 20 cm or more | an exposure evaluation, often a SAR test |
| add a second transmitter that works at the same time | two radios add their emissions and their exposure | a combined evaluation |

## How close is the body?

A transmitter warms the tissue next to it, slightly, and the FCC puts a ceiling on that. Which rule you get depends on a single number: how far the device is from a body when it is used normally.

- **20 cm or more** from the body in normal use. The FCC calls this a mobile device. The check is a calculation of power density, and a low-power sensor passes it on paper.
- **Less than 20 cm.** Now the device is "portable" in FCC language, and what counts is the **SAR**: how many watts each kilogram of tissue absorbs. The ceiling is 1.6 W/kg, averaged over one gram. Hands, wrists, feet and ankles are allowed more, 4 W/kg over ten grams.

Nobody wants a SAR test, because it takes time and real money, and that makes the exemptions worth learning. A transmitter with 1 mW or less is exempt at any distance. Above that, the exempt power depends on the frequency and on the distance. At 20 cm and 2.4 GHz it is around 3 W, far above anything a sensor does. At a few millimetres it falls to a few milliwatts. A wearable, or anything that lives in a pocket, is in SAR territory.

Two radios in one product complicate this. If Wi-Fi and LoRa can be on air together, their exposures are added, and the sum is what gets compared with the limit.

The cheap fixes come first. Can the product really stay 20 cm from people? Then design it that way and print the distance in the manual. A wearable cannot, so its first move is less power, and its second is an antenna on the face that points away from the skin. When neither is enough, accept the SAR test early and put it in the budget and the schedule.

## The label and the manual.

The paperwork that the buyer sees is short, and labs check it.

- The **FCC ID** has to be on the product itself, somewhere visible, and it has to last as long as the product. Is there a screen? Then a menu can show the ID, on the condition that a user gets to it in three steps or fewer.
- A fixed statement goes with it. It repeats the two conditions of Part 15.
- The manual warns that changes not approved by the manufacturer can void the right of the user to operate the device.
- The manual carries the statement for a Class B digital device.

The FCC ID itself has two parts. The first three or five characters are the code of your company, which the FCC assigns. The rest, up to 14 characters, you choose for each product.

## After the grant: what you can change.

A grant describes one product. Later changes fall in three groups.

| Change | Example | What you do |
|---|---|---|
| does not affect the radio | a new colour, a new sensor on the board | nothing; keep your own record |
| makes the radio worse but still legal | a new antenna, a new enclosure that detunes it | file the new measurements before you sell it |
| changes the frequency, the modulation or the maximum power | a new radio chip | a new FCC ID |

Firmware deserves its own warning. The grant covers the radio settings that were tested. Your firmware must not let anybody use the radio outside them. Three locks do most of the job. Nobody edits the power tables. The country is not a choice in the user interface. And at 5 GHz, radar detection has no off switch.

Products that invite people to write their own code have to think harder about this than anybody. Somebody loads an application that you never saw, and the radio limits must hold anyway. The design that survives this keeps the radio stack somewhere the application cannot reach, for example on a second chip.

## Who made your parts: the Covered List.

Until a few years ago the FCC only asked what your device emits. Now it also asks who built it. The **Covered List** names companies and kinds of equipment that the United States considers a security risk, and equipment on that list cannot get a grant at all.

The list has grown quickly.

| Since | What is on the list |
|---|---|
| 2021 | telecom and video surveillance equipment from Huawei, ZTE, Hytera, Hikvision and Dahua |
| December 2025 | radio modules from those companies, and any device that contains one |
| December 2025 | drones produced abroad, and their critical parts |
| March 2026 | consumer routers produced abroad |
| July 2026 | power inverters and advanced robotic devices produced abroad |
| **13 October 2026** | any device that contains a logic component produced by one of the listed companies |

Look at the last row. A "logic component" is any chip or module with digital circuits. So from that date, one integrated circuit from a listed company anywhere in your design blocks the whole product. Grants that already exist are not affected.

The same thinking reached the test labs. Since September 2025, a lab or a TCB that is owned or controlled by a prohibited entity cannot test for the FCC.

None of this needs a lawyer on day one. It needs an afternoon with the bill of materials, to learn who really manufactures each chip, because the name on the reel is sometimes a reseller. It needs one email to your lab, to confirm that the FCC still recognises it. And if your product moves traffic for a home network, it needs a careful reading of the router entry, since a hub and a router can look alike to a regulator.

## And security?

Here the United States and Europe part ways. The FCC has **no mandatory security rule** for a product like the sensor. Nothing in Part 15 asks for signed updates or forbids a default password.

There is a voluntary label, the **U.S. Cyber Trust Mark**. The FCC created the framework in 2024. As of October 2026 it has not really started. The company that was going to run it walked away at the end of 2025, a replacement was named in April 2026, and the labs and administrators were still being approved in the autumn.

So if you sell in both markets, the European rules decide your security design. [What does the CRA ask of an IoT device?](what_does_the_cra_ask_of_an_iot_device.md) covers them.

## The limits and the usual answers.

| What stops you | The usual answer |
|---|---|
| the signal leaks past 2483.5 MHz | lower power on the highest channels, with a power table per channel |
| harmonics fall in restricted bands | a low-pass filter, a shield, and the reference layout of the chip |
| a narrow LoRa signal at 915 MHz | hop over 50 channels or more and respect the 0.4 s on each one, or use the 500 kHz channels |
| the antenna is not the one in the grant | use the granted type and gain, or pay for a new filing |
| the device is used on the body | keep 20 cm, or lower the power, or plan a SAR test |
| two radios transmit at the same time | evaluate them together, or make the firmware take turns |
| the device can run while it charges from the mains | the conducted emission test applies, although it is a battery device |
| users can load their own firmware | lock the power tables and the region where the application cannot reach them |
| a chip from a listed company | check the bill of materials before the layout, not after |

## How it differs from Europe.

| | United States (FCC) | European Union (RED) |
|---|---|---|
| Who approves a transmitter | a TCB, before you sell | you do, with a declaration |
| What you get | an FCC ID | the CE mark |
| How power is limited at 2.4 GHz | 1 W into the antenna | 100 mW out of the antenna |
| Sub-GHz band | 902 to 928 MHz, with hopping and dwell time | 863 to 870 MHz, with duty cycle |
| Radio modules | a module grant carries over to the host | no formal module approval |
| Security | nothing mandatory | mandatory, see the other notes |

[What does the RED ask of an IoT device?](what_does_the_red_ask_of_an_iot_device.md) is the European side of this table.

## Easy to get wrong.

- **"The module is certified, so my product is certified."** The module grant covers the transmitter, under its own conditions. The rest of the product, the label and the antenna are still yours.
- **"The limit is 1 W and I transmit 10 mW, so I pass."** The power limit is not what fails. The band edge and the harmonics are, and they have nothing to do with how far you are from 1 W.
- **"A better antenna is a free upgrade."** More gain, or a different type, takes you outside the grant.
- **"A battery device skips the conducted emission test."** Only if it cannot work while it is plugged in.
- **"The CE mark shows that it would pass the FCC."** The bands, the limits and the procedure are all different.
- **"An FCC grant covers Canada."** Canada has its own certification and its own number.

## Related.

- [What rules does an IoT device have to pass before you can sell it?](what_rules_does_an_iot_device_have_to_pass.md) — the map of all three sets of rules
- [What does the RED ask of an IoT device?](what_does_the_red_ask_of_an_iot_device.md) — the radio rules of the EU
- [What does the CRA ask of an IoT device?](what_does_the_cra_ask_of_an_iot_device.md) — the security rules of the EU
- [How do you build the rules into a device?](how_do_you_build_the_rules_into_a_device.md) — the mechanisms that answer these rules, on real chips

!!! note "Reference"
    The rules are in Title 47 of the Code of Federal Regulations, and the
    [eCFR](https://www.ecfr.gov/current/title-47/part-15) always shows the
    current text. The sections used in this note are 15.203, 15.205, 15.209,
    15.212, 15.225, 15.247, 15.249 and 15.407 for the radio, and 1.1307, 2.1091
    and 2.1093 for exposure. The FCC publishes the
    [Covered List](https://www.fcc.gov/supplychain/coveredlist) and the status
    of the [Cyber Trust Mark](https://www.fcc.gov/CyberTrustMark).
