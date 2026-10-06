# What does the CRA ask of an IoT device?

## Introduction.

For years, security in a connected product was a decision of the company that made it. Some companies cared and many did not, and nobody stopped a device with the password `admin` from reaching the shops. The **Cyber Resilience Act** ends that in the European Union. On paper it is Regulation (EU) 2024/2847. In every meeting it is just the CRA.

The idea fits in one sentence. If you sell something with a processor and a data connection in the EU, it has to be secure when it leaves the factory, you have to keep it secure for years, and you have to tell the authorities when somebody attacks it. All of that becomes part of the CE mark, next to electrical safety and radio.

To keep this concrete, picture one device for the whole note. It is a small battery sensor. Inside there is a Wi-Fi and Bluetooth LE module. New firmware reaches it over the air, and its readings go to a small cloud service that you wrote yourself. Every rule below is read with that device in mind.

!!! note "Versions"
    This note describes the rules as they stand in **October 2026**. Two of the
    four dates of the CRA have already passed and the technical standards are
    still being written, so some of this will age. I am an engineer, not a
    lawyer. Use the note as a map, and read the
    [regulation itself](https://eur-lex.europa.eu/eli/reg/2024/2847/oj) before
    you decide anything that costs money.

## The calendar.

The CRA does not start on one day. It starts in four steps, and two of them are behind you already.

| Date | What starts |
|---|---|
| 10 December 2024 | The regulation is in force. Nothing is required of you so far. |
| 11 June 2026 | The rules for notified bodies, the organisations that will assess the riskier products. |
| **11 September 2026** | **The duty to report.** If your product is attacked, you report it. |
| **11 December 2027** | Everything else: the security requirements, the documentation and the CE mark. |

Two details of this calendar catch people.

The first one is about products that are already in the shops. The security requirements only reach a product that you place on the market from 11 December 2027, or an earlier product that you change in a substantial way after that date. The duty to report is different. It covers **every product in scope that is already on the market**, the old ones included.

The second one is about old designs. Suppose you designed the sensor in 2025 and you are still building it in 2028. Each unit that you first sell after 11 December 2027 has to comply, even though the design is older than the law.

## Is your device in scope?

The CRA talks about a "product with digital elements". That is any hardware or software product that has a data connection to another device or to a network. The connection can be direct or indirect, and it can be a wire or a radio. So the sensor is in, its firmware is in, and the phone app that you ship with it is in.

Here is the part that surprises firmware teams: **your cloud can be in scope too**. The CRA calls it "remote data processing". It counts when two things are true. You designed that software, or somebody designed it for you. And without it the product cannot do one of its functions. The service that receives the readings of the sensor is a clear case. A website that only sells the sensor is not.

What if your service runs on servers that you rent? The service is still yours, so it is in scope. The company that rents you the servers is treated like any other supplier.

A few kinds of product are left out, because they have their own laws: medical devices, cars, certified aircraft parts, marine equipment and anything made only for defence.

Open source deserves a line of its own. Software that somebody publishes without making money from it is not a product, so its author owes nothing. But the moment **you** put that library inside a sensor and sell it, the library is part of your product and the duty is yours.

## Which class is it?

The CRA sorts products by how much damage they can do. The class does not change what the product must do. It changes **who checks it**.

| Class | Examples | Who checks |
|---|---|---|
| Default | most products: a sensor, a smart plug, a speaker | you do, by yourself |
| Important, class I | routers, operating systems, boot managers, microcontrollers with security functions, smart locks, security cameras, baby monitors, connected toys, wearables that monitor health | you do, but only if you follow the official standards in full |
| Important, class II | firewalls, hypervisors, tamper-resistant microcontrollers | a notified body, always |
| Critical | smart cards, secure elements, smart meter gateways | a notified body or a European certificate |

Read the class I row again, because half of it describes the parts list of the sensor. Its microcontroller has secure boot, so it is a "microcontroller with security functions". Its RTOS counts as an operating system. Its bootloader counts as a boot manager. Its Wi-Fi controller counts as a network interface. Does that make the sensor class I?

It does not, and this is the rule to remember: **the class comes from the main function of the product**, not from the parts inside it. A sensor that measures temperature is a default product, whatever chips it uses. The same board sold as a door lock is class I.

Not sure where your product falls? The categories are described one by one in a separate text, Implementing Regulation (EU) 2025/2392, and that is where a doubt gets settled.

## What the product must do.

This is the engineering part. Annex I of the CRA has thirteen requirements for the product itself. None of them names a technology. What each one names is a result. How you reach it is your choice, and you are expected to make that choice after a risk assessment and not before.

In the table, the left column is the requirement in plain words and the right column is what most teams end up doing.

| The CRA asks for | The usual answer in an IoT device |
|---|---|
| no known exploitable vulnerability on the day you ship | scan every dependency for CVEs before each release, and keep the result |
| a secure default configuration, and a way back to it | no shared default password; a unique secret per device or a pairing step at first use; a factory reset |
| security updates, automatic by default, with a way to opt out and to postpone | signed over-the-air updates with two firmware slots; the automatic setting is on when the device leaves the box |
| protection against unauthorised access | authentication on every interface: the local API, the Bluetooth service, the serial console |
| confidential data, stored and in transit | TLS for the network; encrypted flash for keys and user data |
| integrity of the firmware, the commands and the configuration | secure boot, signed images and protection against rollback |
| only the data that the product needs | do not collect what you will not use |
| the essential function keeps working, also under attack | a watchdog, rate limits, and a main function that survives without the network |
| no harm to other devices or networks | back-off with jitter when the server is down, so a fleet does not flood it |
| a small attack surface, external interfaces included | JTAG and UART closed in production; unused services and ports off |
| mitigation, so that one bug does less damage | stack protection, memory protection, least privilege between tasks |
| a record of security events, which the user can switch off | a small security log: failed logins, failed updates, changes of configuration |
| a way to erase all data and settings for good | a factory reset that really erases the keys and the user data |

Two of these rows end products more often than the rest, so look at them twice.

**The update row.** A device with no update mechanism is very hard to defend. The whole regulation assumes that a vulnerability found next year can be fixed next year, in devices that are already installed. You can argue in your risk assessment that this does not apply to your product, but it is a hard argument to win. And if your flash only has room for one firmware image, change the partition table now, while it is still cheap.

**The interface row.** A debug port that is open in production is an external interface with no access control. Teams leave it open because it is useful when a unit comes back from a customer. Does the CRA forbid a debug port? No. What it will not accept is a debug port that anybody can use, so you either burn the fuse that locks it or you put authentication in front of it.

## What you must do.

The second half of Annex I is not about the device. It is about your company, and it lasts for the whole support period. There are eight duties.

1. **Know what is inside.** You keep a list of every software component in the product, the famous SBOM, in a format that a machine can read. Top-level dependencies are the minimum. No format is imposed, and in practice you will pick between SPDX and CycloneDX.
2. **Fix what you find, and do not sit on it.** When you can, the security fix travels alone and does not wait for the next batch of features.
3. **Keep testing.** One security review before the launch is not enough. The CRA expects tests at regular intervals.
4. **Say what you fixed.** After the fix is out, you publish what the vulnerability was.
5. **Have a disclosure policy.** A researcher who finds a bug in your sensor should know what will happen next, because you wrote it down.
6. **Give a contact address** for those reports.
7. **Distribute updates securely.**
8. **Give security updates for free.**

For a small team this sounds like a department. It is closer to four habits. The build generates the SBOM on every release. A job compares that SBOM with the public vulnerability lists every night. The website has a `security.txt` file with an address that somebody reads. And the release process can ship a fix without waiting for the next feature.

One clarification, because people get it wrong: you do **not** have to publish the SBOM. It goes into your technical documentation, and you hand it to an authority that asks for it.

## How long you have to support it.

The support period is **five years at least**. There is one way to have less: if people use the product for less time than that, the support period can match that time. And if the product lasts longer than five years, the period has to be longer. Five years is the floor.

You choose the period, you write the reason down, and you print the end date, with month and year, where the buyer can see it before buying.

A second number hides behind the first one. Every security update that you publish has to stay available for **ten years**, or until the support period ends if that is later.

Think about what that means for the sensor. The radio module needs an SDK that still gets fixes in 2032. The build has to be reproducible years from now, with compilers that still run. And the key that signs the firmware has to survive for a decade, in a place that is safer than a laptop.

## When something goes wrong: 24 hours.

This is the part that already applies. Since 11 September 2026 you have to report two things:

- a vulnerability in your product that somebody is **actively exploiting**;
- a **severe incident** that affects the security of your product.

The clock is short.

| Deadline | What you send |
|---|---|
| 24 hours after you know | an early warning |
| 72 hours after you know | a full notification |
| 14 days after a fix exists | the final report, for a vulnerability |
| 1 month after the notification | the final report, for an incident |

You send them through one website, the single reporting platform that ENISA opened on 11 September 2026. The report reaches the national security team of the country where your company is based, and ENISA sees it at the same time. You also have to tell the users who are affected.

There is one piece of relief for small companies. A micro or small enterprise is not fined for missing the 24-hour deadline. It still has to report, like everybody else.

Twenty-four hours is not enough time to invent a process, so the process has to exist before the bad day. Start with the fleet: if you do not know which firmware version each device runs, you cannot even say who is affected. Then write down who files the report, with a name and a backup. And make sure a fix can go out fast, which brings you back, once more, to the update mechanism.

## Proving it.

From 11 December 2027 the sensor needs a CE mark that covers the CRA. How you get there depends on the class.

- **Default class.** You assess the product yourself. No third party is involved.
- **Class I.** You can assess it yourself only if you apply the official harmonised standards in full. If you do not, a notified body has to check it.
- **Class II and critical.** A notified body, or a European cybersecurity certificate.

Now the catch of October 2026. Those harmonised standards are **not finished**. European standards bodies are writing about forty of them, and none has been cited in the Official Journal so far. Until one is, a class I product has no way to use the self-assessment route. A default product is not blocked by this. Its manufacturer can go straight to the text of Annex I and assess the product against it. A guidance document that the Commission published in July 2026, with 67 worked examples, helps with that.

No class escapes the technical file. Think of it as the folder you would hand to an inspector: what the product is, the risk assessment, the SBOM, the disclosure policy, why you chose that support period, and the test reports. It has to stay on your shelf for ten years at least.

The buyer gets a smaller folder. In the box or on your website there has to be a way to contact you, the address for vulnerability reports, the date when support ends, and instructions for two moments: installing updates, and wiping the device before it changes hands.

## What it costs to ignore it.

| What went wrong | Maximum fine |
|---|---|
| the product fails the security requirements, or you fail the manufacturer duties or the reporting | 15 million euros, or 2.5 % of worldwide turnover |
| other duties, such as the documentation and the CE mark | 10 million euros, or 2 % |
| false information to an authority | 5 million euros, or 1 % |

In each row the higher of the two numbers applies. An authority can also do something that hurts more than a fine: it can order the product off the market.

## How it fits with the other rules.

The CRA is not the first EU rule about the security of connected devices. Since 1 August 2025, radio products have had to meet security requirements under the Radio Equipment Directive. That piece of the RED ends on 11 December 2027, the same day the CRA starts in full, and the CRA takes over. If you build for the RED rules now, you are most of the way there. [What does the RED ask of an IoT device?](what_does_the_red_ask_of_an_iot_device.md) has the details.

Outside the EU the picture is different. The United Kingdom has its own, much shorter law. The United States has nothing mandatory of this kind for a sensor, as [What does the FCC ask of an IoT device?](what_does_the_fcc_ask_of_an_iot_device.md) explains.

## Easy to get wrong.

- **"The CRA starts in December 2027."** The reporting duty started on 11 September 2026, and it covers products that are already in the shops.
- **"My module is certified, so I am covered."** You are the manufacturer of the final product, and the duty is yours. The CRA asks you to check your suppliers. It does not let you hide behind them.
- **"A secure microcontroller makes my product class I."** The main function of the product decides the class. The parts do not.
- **"The cloud is a different product."** If you built it and the device needs it, it is part of the product.
- **"Five years of support is the rule."** Five years is the minimum. A product that lives longer needs more.
- **"I have to publish my SBOM."** You have to keep it and show it to an authority that asks.
- **"Open source is exempt."** The volunteer who wrote the library is. You, who sell it inside a device, are not.

## Related.

- [What rules does an IoT device have to pass before you can sell it?](what_rules_does_an_iot_device_have_to_pass.md) — the map of all three sets of rules
- [What does the RED ask of an IoT device?](what_does_the_red_ask_of_an_iot_device.md) — the radio rules of the EU, and the security rules that the CRA replaces
- [What does the FCC ask of an IoT device?](what_does_the_fcc_ask_of_an_iot_device.md) — the radio rules of the United States
- [How do you build the rules into a device?](how_do_you_build_the_rules_into_a_device.md) — the mechanisms that answer these rules, on real chips

!!! note "Reference"
    The text of the regulation is on
    [EUR-Lex](https://eur-lex.europa.eu/eli/reg/2024/2847/oj). Annex I has the
    requirements, Annex III and Annex IV have the classes, and Article 14 has
    the reporting deadlines. The Commission keeps a
    [page on the CRA](https://digital-strategy.ec.europa.eu/en/policies/cyber-resilience-act)
    with the guidance and the state of the standards, and ENISA runs the
    [reporting platform](https://www.enisa.europa.eu/news/the-cra-single-reporting-platform-is-launched).
