# What rules does an IoT device have to pass before you can sell it?

## Introduction.

You have a prototype that works. It connects, it sends its data, the battery lasts. Between that prototype and a shop there is a wall of rules, and most engineers meet it late, when the board is already laid out and the firmware is nearly done.

That is the expensive moment to meet it. A rule that asks for a second firmware slot costs nothing in the first week of a design. In the last week it means a new partition table and an update that old units cannot take.

This note is the map. It covers the three sets of rules that decide the fate of a connected device in the two biggest markets, and it ends with the design decisions that satisfy all three at once. Each rule then has a note of its own.

The device is the same in the whole section: a small battery sensor with a Wi-Fi and Bluetooth LE module, sometimes a LoRa radio, firmware updates over the air, and a small cloud service behind it.

!!! note "Versions"
    This note describes the rules as they stand in **October 2026**. I am an
    engineer, not a lawyer. The notes of this section are a map for design
    decisions. They do not replace the official texts, or the test lab that
    will sign your reports.

## The three rules, side by side.

| | RED | CRA | FCC |
|---|---|---|---|
| Where | European Union | European Union | United States |
| What it is | a directive for radio equipment | a regulation for anything with software and a data connection | the federal rules for radio emissions |
| What it worries about | interference, safety, and security for now | security, for the whole life of the product | interference, and who made the parts |
| Who checks before you sell | you do | you do, for most products | an outside body, always, for a transmitter |
| What you get | the CE mark | the same CE mark | an FCC ID |
| Applies since | 2016 for radio; 1 August 2025 for security | 11 September 2026 for reporting; 11 December 2027 for the rest | decades for radio; the rules about suppliers are recent |

Three things in that table are worth a second look.

**Two of the three are European, and they overlap for a while.** The RED has had security requirements since August 2025. The CRA replaces them on 11 December 2027 with a longer list. If you launch a product in 2026, you design for the RED and you plan for the CRA.

**Europe trusts you first and checks later.** In the EU you declare by yourself that the product complies. In the United States a transmitter needs an approval number before the first unit is sold. Neither is easier. The European way moves the risk to the day an authority tests a product that is already in the shops.

**Only Europe makes security mandatory.** The FCC cares about what your device emits and, since a few years ago, about which companies made its chips. It has no rule against a default password.

## What each rule limits.

The same sensor meets very different limits on each side of the Atlantic.

| Subject | In the EU | In the United States |
|---|---|---|
| power at 2.4 GHz | 100 mW radiated, antenna gain included | 1 W into the antenna |
| what fails at 2.4 GHz | the sharing rules above 10 dBm | the edge of the band at 2483.5 MHz |
| sub-GHz band | 863 to 870 MHz | 902 to 928 MHz |
| sub-GHz rule | a duty cycle, often 1 % | hopping, and 0.4 seconds per channel |
| the receiver | tested | not tested |
| immunity to interference | tested | not tested |
| exposure limit on the body | 2 W/kg over ten grams | 1.6 W/kg over one gram |
| a certified radio module | helps, but the product is assessed again | carries its approval into the product |
| security of the product | mandatory | voluntary |
| support and updates | five years or more, from December 2027 | no rule |
| reporting an attack | within 24 hours, since September 2026 | no rule |
| origin of the chips | no rule | no chip from a listed company |

Nothing in this table can be fixed by a clever region setting alone. The sub-GHz row means two different radios, or one radio with two firmware behaviours. The exposure row means two different tests. Plan for both from the start, or pick one market and say so.

## Design once for all three.

Here is the good news. A short list of decisions, taken early, clears most of the three rules together. None of them is exotic, and [How do you build the rules into a device?](how_do_you_build_the_rules_into_a_device.md) shows what each one looks like on real chips.

1. **Buy a radio module with an FCC grant and with test reports for the RED.** Then use it exactly as it was tested: its antenna, its power settings, its distance to the body.
2. **Keep a power table per region and per channel, and lock it.** Europe caps the radiated power. The United States needs lower power on the highest channels. Both rules forbid the user from changing it.
3. **Make the region a factory setting.** The duty cycle of Europe and the hopping of the United States cannot live behind a menu option.
4. **Decide how close the device is to the body.** At 20 cm or more, and at low power, both exposure rules are paperwork. On the wrist or in a pocket, both mean a test.
5. **Give the flash two firmware slots, and sign every update.** The RED asks for authenticated updates and the CRA asks for updates at all. Add protection against rollback while you are there.
6. **Ship no default password and no optional one.** A unique secret per device, or a setup that cannot finish without one. In Europe, the optional password alone sends you to a notified body.
7. **Close the debug ports in production**, or put authentication in front of them.
8. **Generate a list of your software parts on every build**, and compare it with the public vulnerability lists. The CRA asks for it, and it is the only way to answer a 24-hour deadline.
9. **Publish an address for security reports**, and make sure somebody reads it.
10. **Choose the support period before you choose the parts.** Five years is the floor in Europe. The radio module, its SDK and your signing key all have to live that long.
11. **Check who really makes each chip.** One integrated circuit from a company on the American list blocks the whole product there.
12. **Write the technical file as you go.** Every rule here ends in a folder of documents. It is far easier to fill it during the design than to rebuild it afterwards.

## When each decision gets expensive.

The same decision costs very different amounts depending on the day you take it.

| Stage | What you can still change cheaply | What is already expensive |
|---|---|---|
| choosing parts | the module, the antenna, the chip vendors, the flash size | nothing yet |
| before the layout | antenna position, filters, test points, debug port protection | a different module |
| before the firmware freezes | power tables, airtime budget, update mechanism, passwords | the flash size, the antenna |
| at the test lab | a few decibels of power, the text of the manual | everything else |
| after the launch | security fixes, through the update mechanism you built | whatever has no update path |

The last row is the one to remember. After the launch, the update mechanism is the only tool you have left. That is why the CRA is built around it.

## Where to read next.

- [What does the RED ask of an IoT device?](what_does_the_red_ask_of_an_iot_device.md) — radio limits in Europe, the duty cycle at 868 MHz, and the security requirements that apply now
- [What does the CRA ask of an IoT device?](what_does_the_cra_ask_of_an_iot_device.md) — security by design, the support period, and the 24-hour report
- [What does the FCC ask of an IoT device?](what_does_the_fcc_ask_of_an_iot_device.md) — radio limits in the United States, modules, and the list of banned suppliers
- [How do you build the rules into a device?](how_do_you_build_the_rules_into_a_device.md) — the same decisions with real option names, on ESP32, STM32 and Nordic chips

## Easy to get wrong.

- **"Compliance is the last step."** It is a set of design decisions. Most of them are cheap in the first month and expensive in the last.
- **"One certification covers the world."** The CE mark and the FCC ID share almost nothing: different bands, different limits, different procedures.
- **"The module vendor did the work."** The vendor did the work for the module. The product is yours in all three rules.
- **"Security rules arrive in 2027."** In Europe, security has been mandatory for radio products since August 2025, and the reporting duty since September 2026.
- **"These rules are for big companies."** They apply to whoever puts the product on the market. A small company gets a little relief in the CRA, and none in the radio rules.
