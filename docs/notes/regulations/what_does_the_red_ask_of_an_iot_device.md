# What does the RED ask of an IoT device?

## Introduction.

The **Radio Equipment Directive** is the European law for anything that transmits or receives radio on purpose. Its number is 2014/53/EU and everybody calls it the RED. If your product has Wi-Fi, Bluetooth, LoRa, NFC or any other radio, and you want to sell it in the European Union, this is the rule that stands between the prototype and the CE mark.

The first thing to understand is how different the European system is from the American one. Before you sell, no office looks at your product and no number is issued. The whole process is in your hands: the tests, a declaration that says the product complies, and the CE mark. So is the responsibility. That sounds easy, and it is easy to do badly. Authorities buy products in shops and test them, and a product that fails can be pulled from the whole market.

The device of this note is the same sensor as in the other notes of this section. It runs on a battery, it has a Wi-Fi and Bluetooth LE module at 2.4 GHz, and sometimes it carries a LoRa radio at 868 MHz.

!!! note "Versions"
    This note describes the rules as they stand in **October 2026**. The radio
    limits change slowly, but the list of official standards changes every few
    months, and the security part of the RED has an end date. I am an engineer,
    not a lawyer. Use the note as a map, and check the
    [directive](https://eur-lex.europa.eu/eli/dir/2014/53/oj) and your test lab
    before you decide anything that costs money.

## Three requirements, and a few extras.

The whole directive hangs from one article, Article 3. It asks for three things from every radio product.

1. **Health and safety.** The product must not hurt anybody. That covers electrical safety, the battery, and the radio energy that a body absorbs.
2. **Electromagnetic compatibility.** The product must not disturb other equipment, and it must keep working when other equipment disturbs it.
3. **Good use of the spectrum.** The radio must stay inside its band and its power, and must not cause harmful interference.

Article 3 then has a list of **extra requirements**. They sleep until the Commission switches one of them on for a class of products. Three switches matter to an IoT device.

| Switched on | For | Since |
|---|---|---|
| security: protect the network, protect personal data, prevent fraud | radio products that can reach the internet | 1 August 2025 |
| a common charger: USB-C | phones, tablets, cameras, headphones, earbuds, handheld consoles, portable speakers, e-readers, keyboards, mice and navigation devices | 28 December 2024 |
| the same common charger | laptops | 28 April 2026 |

The sensor is not in the charger list, and neither is a smartwatch. The security row is another story, and it has its own section below.

## How you prove it: harmonised standards.

The directive never gives a number. It says "no harmful interference" and stops there. The numbers live in **harmonised standards**, the technical documents that ETSI and other bodies write and the Commission publishes in the Official Journal.

A harmonised standard gives you something valuable, called presumption of conformity. If your product passes the standard, the law presumes that it meets the requirement. You are allowed to prove compliance some other way, but then a notified body has to agree with you, and that costs time and money.

So in practice the standards are the rules. This is the short list for an IoT device.

| Requirement | Standard | What it covers |
|---|---|---|
| spectrum | EN 300 328 | 2.4 GHz: Wi-Fi, Bluetooth, Zigbee, Thread |
| spectrum | EN 301 893 | 5 GHz Wi-Fi |
| spectrum | EN 303 687 | 6 GHz Wi-Fi |
| spectrum | EN 300 220 | short range devices below 1 GHz: LoRa and others |
| spectrum | EN 300 330 | NFC at 13.56 MHz, RFID at 125 kHz |
| compatibility | EN 301 489 | emissions and immunity of radio products |
| safety | EN 62368-1 | electrical safety |
| safety | EN 62479, EN 50566 and others | exposure to radio energy |
| security | EN 18031 | the three security requirements |

## What each band allows.

| Band | Used by | Main limits |
|---|---|---|
| 2400 to 2483.5 MHz | Wi-Fi, Bluetooth, Zigbee, Thread | 100 mW (20 dBm) e.i.r.p. |
| 5150 to 5350 MHz | Wi-Fi | 200 mW (23 dBm) e.i.r.p.; mostly for indoor use, and the upper half indoors only |
| 5470 to 5725 MHz | Wi-Fi | 1 W (30 dBm) e.i.r.p., with radar detection and power control |
| 5945 to 6425 MHz | Wi-Fi 6E | 200 mW indoors only; 25 mW (14 dBm) for portable devices |
| 863 to 870 MHz | LoRa and other sub-GHz radios | 25 mW e.r.p. in most of it, with a duty cycle |
| 13.56 MHz | NFC | 42 dBµA/m at 10 m; 60 dBµA/m for NFC and RFID readers |
| 119 to 135 kHz | RFID | 66 dBµA/m at 10 m at the bottom of the band, less above it |

Two details of this table decide most designs.

**The limit is e.i.r.p.** That is the power that leaves the antenna, with the gain of the antenna included. A radio chip that delivers 20 dBm into an antenna with 3 dBi of gain radiates 23 dBm, and that is over the limit at 2.4 GHz. So the transmit power in your firmware is the limit minus the gain of your antenna. A better antenna forces you to turn the transmitter down.

**The receiver is tested too.** The RED checks that your receiver keeps working when a strong signal appears just outside the band. This test is called blocking. A cheap receiver with no filter in front can fail it, even if the transmitter is perfect.

## 2.4 GHz: the line at 10 dBm.

EN 300 328 draws a line at **10 dBm e.i.r.p.**, and the two sides of it are different worlds.

Below 10 dBm, the standard leaves you alone. There is no rule about sharing the channel. This is where most Bluetooth LE, Zigbee and Thread devices live, and it is one more reason to keep their power modest.

Go above 10 dBm and the standard expects your radio to be a good neighbour. It can do that in one of two ways.

- **Adaptive.** Before each transmission the radio checks whether somebody else is using the channel, and if so it waits. Wi-Fi has worked like this from its first version, so a Wi-Fi module gives it to you with no effort.
- **Not adaptive.** A radio that never listens is still allowed, as long as it does not hog the channel. The measure is simple. Take the power as a fraction of 100 mW, multiply it by the fraction of time you are on air, and keep the product under 10 %. A transmitter at the full 100 mW may therefore be on air one tenth of the time.

Power density is the other thing to watch. Unless your radio hops, it may not put more than 10 dBm into any single megahertz. That is bad news for a narrow signal: a carrier that is 1 MHz wide is already at the limit with 10 dBm, and raising it to 20 dBm is not an option.

## Below 1 GHz: the duty cycle.

The main European band below 1 GHz is only 7 MHz wide, from 863 to 870 MHz, and thousands of products want to use it. The solution was to ration time. Each device may talk for **a small part of every hour** and no more, and if you build anything with LoRa this is the rule you will think about most.

| Sub-band | Maximum power (e.r.p.) | Duty cycle |
|---|---|---|
| 863 to 865 MHz | 25 mW | 0.1 % |
| 865 to 868 MHz | 25 mW | 1 % |
| 868.0 to 868.6 MHz | 25 mW | 1 % |
| 868.7 to 869.2 MHz | 25 mW | 0.1 % |
| 869.4 to 869.65 MHz | 500 mW | 10 % |
| 869.7 to 870 MHz | 25 mW | 1 % |

Percentages like these are hard to picture until you turn them into seconds. One hour has 3600 seconds, so 1 % of it is **36 seconds of transmission**. The 0.1 % sub-bands give you 3.6 seconds in an hour, and the generous one at 10 % gives you six minutes.

Now put a real message in it. A LoRaWAN reading of 10 bytes at the slowest data rate stays on air for about 1.5 seconds. With 36 seconds per hour, that is 24 messages per hour and not one more. Send them faster and the device breaks the law, however well the firmware works.

If you know LoRaWAN, you can now see why its European plan looks the way it does. The three channels that every device must support are 868.1, 868.3 and 868.5 MHz, all of them inside the 1 % sub-band. And the second receive window of every device listens on 869.525 MHz. That frequency was not picked at random: it is in the only sub-band where a transmitter may use 500 mW and 10 % of the time.

The standard does offer a way out of the duty cycle. It is called polite spectrum access, and it means that the radio listens first and moves to another channel when the one it wanted is busy. Few sub-GHz products bother with it, and LoRaWAN is not one of them.

So how do real products live inside 36 seconds per hour? With four habits, and most products need all of them.

- The firmware keeps an **airtime budget** per sub-band and refuses to send when the budget is spent.
- The device uses the fastest data rate that the link allows, because a faster message is a shorter message.
- The traffic is spread over several sub-bands, since each one has its own budget.
- The application is designed for it from the start: fewer messages, smaller messages, and no retries in a tight loop.

## 5 GHz: indoors, and away from radar.

Wi-Fi at 5 GHz is a guest in somebody else's band. Weather radar and military radar were there first, and the conditions are written to protect them. Part of the band is for indoor use only. In two sub-bands the network has to detect radar and move away from it. In most networks the access point does that work, and a client device follows it.

For the firmware of a client, the rule comes down to one thing. It has to know which country it is in, and that answer cannot come from a menu that the user controls.

## Safety and exposure.

Two checks sit under the safety requirement.

The first is plain electrical safety: the battery, the charger input, the temperature of the case. EN 62368-1 covers it.

The second is the radio energy that the body absorbs, the **SAR**. The European limit is 2 W/kg, averaged over ten grams of tissue, for the head and the trunk. For the limbs it is 4 W/kg.

There is an exemption that most small devices use. A transmitter whose average power is 20 mW or less is treated as compliant without a measurement. A Bluetooth LE device at a few milliwatts is far inside it. A Wi-Fi device at 100 mW that is worn on the body is not, and it needs an evaluation.

Compatibility has a second half that surprises people who come from the American rules. Europe tests **immunity**. At the lab somebody will zap your product with static discharges, bathe it in strong radio fields and inject noise into its cables. Through all of that it has to keep working, or at least come back by itself. A device that resets every time a hand touches its USB connector does not pass.

## Security: the three extra requirements.

On 1 August 2025 the RED grew a security chapter. From that day, a radio product that can reach the internet must meet three more requirements, which come from a text called Delegated Regulation (EU) 2022/30.

| Requirement | Applies to |
|---|---|
| do not harm the network or misuse its resources | any radio product that can reach the internet, directly or through other equipment |
| protect personal data and privacy | the same products when they handle personal data, and also toys, childcare products and wearables |
| protect against fraud | products that can move money or virtual currency |

Read "directly or through other equipment" slowly. A sensor that reaches the internet through a gateway or through a phone is the grey case. The text does not settle it, and the safe choice is to assume that the sensor is in.

The standard for these requirements is **EN 18031**, in three parts, one for each row of the table. Put in plain words, it asks for this:

- access to the device is limited to whoever is authorised;
- there is no universal default password, and guessing a password by brute force is hard;
- updates are authenticated and checked for integrity before they are installed;
- secrets are stored protected;
- communication is protected;
- each device has its own keys, and they are protected;
- the cryptography is current good practice.

None of that is exotic. It is the same list that a careful team already follows. The trap is somewhere else.

**The trap: the standard was published with restrictions.** In three situations, passing EN 18031 does not give you the presumption of conformity, and you need a notified body.

| Situation | What it means |
|---|---|
| the user can skip setting a password | a notified body, unless you remove that path |
| a toy or a childcare product without parental access control | a notified body |
| a product that moves money | a notified body, always, for the update requirements |

The first row is the one that catches IoT devices. A setup screen with a "skip" button under the password field is enough. The fix is cheap if you do it early: the device ships with a unique secret printed on its label, or the setup does not finish until the user has set one.

This part of the RED has an end date. On **11 December 2027** it is repealed, and the Cyber Resilience Act takes over with a longer list of requirements. A product that you place on the market before that date stays under the RED rules. [What does the CRA ask of an IoT device?](what_does_the_cra_ask_of_an_iot_device.md) explains what comes next.

## Who checks: you do.

The procedure depends on one question. Did you apply the harmonised standards in full?

- **Yes.** You assess the product yourself. No third party is needed.
- **No, or only in part, or no standard exists.** For the spectrum and the security requirements, a notified body has to examine the product.

Either way, the paperwork is yours, and you keep it for ten years.

| Document | What goes in it |
|---|---|
| technical file | a description with photos, the firmware versions that affect compliance, the design drawings, the list of standards, the test reports |
| EU declaration of conformity | who you are, what the product is, which laws and standards it meets, and your signature |
| the manual | the frequency bands, the maximum radio power in each one, and the safety information |
| the product and the box | the CE mark, a type or serial number, your name and postal address, and a pictogram if some country restricts its use |

If somebody else imports the product into the EU, the importer adds its own name and address, and has to check that you did your part.

## Modules: no shortcut on paper, a big one in practice.

In the United States a certified radio module carries its approval into your product. Europe has nothing like that. People in the trade repeat one formula: **CE + CE ≠ CE**. Put CE-marked parts together and what comes out is a new product with no mark at all, and its manufacturer is you.

That does not make a module useless. It changes what the module gives you.

| Requirement | Can you reuse the tests of the module? |
|---|---|
| spectrum | yes, if you use the module the way it was tested, with the same antenna and power, and its maker gives you the reports |
| compatibility | no; the final product is tested, with the radio working |
| safety and exposure | no; they depend on your enclosure, your battery and how the product is used |
| security | no; it is assessed on the final product |

So when you choose a module, ask for its RED test reports and not only for its declaration. And keep its antenna. A different antenna changes the radiated power and the pattern, and the reports stop describing your product.

## The limits and the usual answers.

| What stops you | The usual answer |
|---|---|
| 100 mW e.i.r.p. at 2.4 GHz, antenna gain included | transmit power set to the limit minus the antenna gain, in a table per region |
| the sharing rules above 10 dBm | stay at 10 dBm or less for simple radios, or use a stack that listens before it talks |
| the duty cycle at 868 MHz | an airtime budget in the firmware, faster data rates, fewer and smaller messages |
| the receiver blocking test | a filter in front of the receiver, and a radio chip with a decent receiver |
| the immunity tests | protection on every connector, and a firmware that recovers by itself |
| exposure on a device worn on the body | 20 mW of average power or less, or a SAR evaluation |
| a password that the user can skip | a unique secret per device, or a setup that cannot finish without one |
| an antenna that is not the one of the module | keep the antenna of the module, or test the radio again |
| 5 GHz indoor and radar rules | the country fixed at the factory, never chosen by the user |

## Easy to get wrong.

- **"The module has a CE mark, so my product has one."** CE + CE ≠ CE. You are the manufacturer of a new product.
- **"Nobody checks, so it does not matter."** Nobody checks before you sell. Authorities check afterwards, and they can remove the product from every shop in the EU.
- **"My radio chip is set to 20 dBm, so I am at the limit."** The limit includes the antenna gain. With a 3 dBi antenna you are 3 dB over.
- **"The duty cycle is a LoRaWAN rule."** It is the rule of the band. It applies to any protocol that transmits there.
- **"Security is a 2027 problem."** The security requirements of the RED have applied since 1 August 2025.
- **"I passed EN 18031, so I do not need a notified body."** Not if your product falls in one of the three restrictions.
- **"The FCC report covers Europe."** Europe measures radiated power, tests the receiver and tests immunity. The American report has none of the three.

## Related.

- [What rules does an IoT device have to pass before you can sell it?](what_rules_does_an_iot_device_have_to_pass.md) — the map of all three sets of rules
- [What does the CRA ask of an IoT device?](what_does_the_cra_ask_of_an_iot_device.md) — the security rules that replace this part of the RED in 2027
- [What does the FCC ask of an IoT device?](what_does_the_fcc_ask_of_an_iot_device.md) — the radio rules of the United States
- [How do you build the rules into a device?](how_do_you_build_the_rules_into_a_device.md) — the mechanisms that answer these rules, on real chips

!!! note "Reference"
    The [directive](https://eur-lex.europa.eu/eli/dir/2014/53/oj) is short, and
    Article 3 is the part to read. The security requirements are in
    [Delegated Regulation (EU) 2022/30](https://eur-lex.europa.eu/eli/reg_del/2022/30/oj),
    and their end date is in
    [Delegated Regulation (EU) 2026/339](https://eur-lex.europa.eu/eli/reg_del/2026/339/oj).
    The ETSI standards are free to download from
    [etsi.org](https://www.etsi.org/standards); the limits of this note come
    from EN 300 328 V2.2.2, EN 301 893 V2.2.1, EN 300 220-2 V3.3.1 and
    EN 300 330 V2.1.1. The Commission keeps the
    [list of harmonised standards](https://single-market-economy.ec.europa.eu/sectors/electrical-and-electronic-engineering-industries-eei/radio-equipment-directive-red_en)
    on its page for the directive.
