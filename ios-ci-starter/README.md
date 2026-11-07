# iOS CI starter — build and run on your iPhone with no Mac and no paid account

Write Swift on Windows → build on GitHub's Apple Silicon runners → sideload onto
your iPhone with Sideloadly using a **free** Apple ID.

No Mac. No Xcode locally. No $99 Apple Developer Program. No Hyper-V VM.

## How it fits together

```
Windows (edit Swift)  →  GitHub Actions (xcode-27 runner)  →  unsigned .ipa artifact
                                                                      ↓
                              iPhone (iOS 27)  ←  Sideloadly on Windows (free Apple ID)
```

- **No `.xcodeproj` in git.** `project.yml` is an [XcodeGen](https://github.com/yonaskolb/XcodeGen)
  spec; CI generates the project. Editing an `.xcodeproj` normally needs Xcode —
  this removes that requirement.
- **No signing in CI.** The build is unsigned, so the workflow needs **zero
  secrets**. Signing happens on your PC, with your free Apple ID, at install time.
- **`runs-on: xcode-27`** is GitHub's Apple Silicon image with Xcode 27 and the
  iOS 27 SDK. (`macos-26` does *not* include Xcode 27.)

## Setup

### 1. Move these files into your app repo

Copy this folder's contents to the repo root so the workflow lands at
`.github/workflows/ios.yml`. It's nested here only to keep it from running in
the repo it was delivered in.

That's the entire CI setup. No secrets, no variables, no Apple account.
Optionally set repository variables `BUNDLE_ID` and `BUNDLE_ID_PREFIX`;
the defaults build fine.

### 2. Get the IPA

Push to `main` (or run the workflow manually) → Actions → the run →
**Artifacts** → `App-ipa-<n>`. Unzip it to get `App-<n>.ipa`.

### 3. Install it with Sideloadly

1. Install [iTunes](https://www.apple.com/itunes/download/win64) (needed for
   Apple's USB drivers) and [Sideloadly](https://sideloadly.io) on Windows.
2. Plug the iPhone in, trust the computer.
3. Drag the `.ipa` into Sideloadly, enter your **free** Apple ID, hit Start.
4. On the phone: Settings → General → VPN & Device Management → trust your
   developer profile. Then launch the app.

Use an app-specific password if your Apple ID has 2FA
(<https://appleid.apple.com> → Sign-In and Security → App-Specific Passwords).

## What free signing costs you

These are Apple's limits on a free Apple ID, not Sideloadly's:

| Limit | Value |
| --- | --- |
| App validity | **7 days**, then it won't launch until re-signed |
| Apps signed at once | **3** |
| New app IDs | ~10 per rolling week |
| Push notifications | Not available |
| Network Extensions, CarPlay, etc. | Not available |
| TestFlight / App Store | Not available |

Re-signing means running Sideloadly again with the phone connected. AltStore or
SideStore can refresh over Wi-Fi instead, if weekly cable duty annoys you.

## Confidence notes

- **iOS 27 support is the vendor's own claim.** Sideloadly's site lists iOS 7
  through "iOS 27+". I found no independent confirmation. Check their release
  notes before trusting it.
- **Free-account soft-bans have been reported.** One blog reports Apple
  soft-banning some free teams via PPQ, producing a banned-profile error. Single
  unconfirmed source, but worth knowing the free tier isn't contractually solid.
- **The CI side is untested.** The YAML is valid and the steps are standard, but
  it has not been run on a macOS runner.

## Costs

Public repo: free. Private repo: macOS runners bill at **10× the Linux rate**, so
2,000 free minutes ≈ **200 macOS minutes/month**. Each run here is a few minutes.
Make the repo public if you can.

## Limitations worth accepting up front

- **No debugger, no breakpoints.** Build logs and test results only.
- **No local Simulator.** It runs headless on the runner.
- **Minutes per iteration, not seconds.** Fine for logic, slow for UI work.

Keep business logic in plain Swift types (like `Greeting` in `Sources/`) and
SwiftUI views thin — that way the untestable-locally surface stays small.

If UI iteration becomes the bottleneck, a used Apple Silicon Mac mini (~$400) is
the fix, not more CI tuning.

## If you ever do get a paid account

A TestFlight job — App Store Connect API key signing, `-allowProvisioningUpdates`,
`altool` upload, no certificate export needed — is in this repo's git history at
commit `19571df`. Recover it with:

```
git show 19571df:ios-ci-starter/.github/workflows/ios.yml
```
