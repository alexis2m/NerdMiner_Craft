# Security Policy

## Reporting a vulnerability

If you believe you have found a security issue in NerdMiner_Craft — for example, a way for a malicious Wi-Fi portal, captive HTTP page, or stratum response to compromise the device or leak the user's BTC address — **please do not open a public issue**.

Instead, email **hello@alexismaison.com** with:

- A description of the issue.
- Steps to reproduce, ideally with the firmware version / commit and the FNK0103 variant you tested on.
- Any logs, captures, or proof-of-concept code (please refrain from sharing exploits publicly while we work on a fix).

You should expect an acknowledgment within **5 business days**. If the issue is confirmed, a fix and a coordinated disclosure timeline will follow.

## Scope

In scope:

- The firmware in this repository (NerdMiner_Craft).
- Wi-Fi captive portal / `WiFiManager` flow.
- Stratum client implementation.
- Build pipeline / GitHub Actions and supply chain (workflow injections, malicious dependencies).

Out of scope:

- Bugs in upstream [BitMaker-hub/NerdMiner_v2](https://github.com/BitMaker-hub/NerdMiner_v2) that also reproduce there — please report those upstream.
- Bugs in third-party libraries — report to the library maintainer.
- The security of any specific mining pool you connect to.
- Loss of mining rewards due to misconfiguration (e.g. wrong BTC address).

## Hardening reminders for users

- The captive portal SSID `MinerAP` ships with a public default password (`MineYourCoins`). It is **only used during initial setup**; once your home Wi-Fi is configured the AP shuts down. Don't leave a device permanently in setup mode on an untrusted network.
- Only the **public** BTC receiving address is needed. Never enter a private key or seed phrase into the device or its captive portal — there is no legitimate reason to.
- Verify the source of any binary you flash. Releases here are signed by tagged commits.
