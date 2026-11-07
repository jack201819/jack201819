# iOS CI starter — build for iOS 27 without owning a Mac

Write Swift on Windows, build on GitHub's Apple Silicon runners, install on your
iPhone through TestFlight. No Mac, no Xcode locally, no VM.

## Why it's shaped this way

- **No `.xcodeproj` in git.** `project.yml` is an [XcodeGen](https://github.com/yonaskolb/XcodeGen)
  spec; CI generates the project. Creating or editing an `.xcodeproj` normally
  requires Xcode — this removes that requirement entirely.
- **No certificates to export.** Signing uses an App Store Connect **API key**
  with `-allowProvisioningUpdates`, so Xcode creates and fetches the
  distribution certificate and profile on the runner. The usual
  "export a .p12 from Keychain Access" step needs a Mac; this doesn't.
- **`runs-on: xcode-27`** is the GitHub-hosted Apple Silicon image carrying
  Xcode 27 and the iOS 27 SDK. (`macos-26` does *not* include Xcode 27.)

## Setup

### 1. Move these files into your app repo

Copy the contents of this folder to the repo root, so the workflow sits at
`.github/workflows/ios.yml`. It's nested here only to keep it from running in
the repo it was delivered in.

### 2. Apple Developer Program

You need a paid membership ($99/yr) — TestFlight is not available on a free
Apple ID. Note your **Team ID** from
<https://developer.apple.com/account> → Membership.

### 3. Create an App Store Connect API key

<https://appstoreconnect.apple.com> → Users and Access → Integrations → App Store Connect API

- Role: **App Manager** (needs to create certificates and upload builds)
- Download the `.p8` **once** — Apple will not let you download it again
- Note the **Key ID** and **Issuer ID**

### 4. Register the app

App Store Connect → Apps → **+** → New App. The bundle ID must match what you
set in step 6. The app record must exist before the first upload.

### 5. Add repository secrets

Settings → Secrets and variables → Actions → **Secrets**:

| Secret | Value |
| --- | --- |
| `ASC_KEY_ID` | the Key ID from step 3 |
| `ASC_ISSUER_ID` | the Issuer ID from step 3 |
| `ASC_KEY_P8` | the `.p8` file, base64-encoded (see below) |
| `TEAM_ID` | your 10-character Team ID |

Encode the key on Windows:

```powershell
[Convert]::ToBase64String([IO.File]::ReadAllBytes("$HOME\Downloads\AuthKey_ABC123.p8")) | Set-Clipboard
```

### 6. Add repository variables

Same page, **Variables** tab:

| Variable | Example |
| --- | --- |
| `BUNDLE_ID` | `com.yourname.yourapp` |
| `BUNDLE_ID_PREFIX` | `com.yourname` |

## Using it

| Event | What runs |
| --- | --- |
| Pull request | Build + unit tests on simulator, no signing, no secrets |
| Push to `main` | Tests, then archive and upload to TestFlight |
| Tag `v*` | Same as `main` |
| Manual run | Tests; ticks the TestFlight box only if you ask |

Build numbers come from `github.run_number`, so every upload is unique —
App Store Connect rejects duplicates.

After upload, the build takes 5–15 minutes to finish processing before it
appears in TestFlight. Install the TestFlight app on your iOS 27 iPhone and
sign in with the same Apple ID.

## Honest limitations

- **No debugger, no breakpoints.** You get build logs and test results.
- **No Simulator on your machine.** It runs on the runner, headless.
- **Round trip is minutes, not seconds.** Fine for logic, slow for UI work.
- **Free-tier minutes:** macOS runners bill at **10×** the Linux rate, so
  2,000 free minutes ≈ **200 macOS minutes/month** on a private repo. Public
  repos are free. Keep the TestFlight job off pull requests (it already is).

If UI iteration becomes the bottleneck, that's the point to buy a used
Apple Silicon Mac rather than fight this.

## Local checks without a Mac

Swift on Windows can typecheck non-UIKit/SwiftUI logic. Keep your business
logic in plain Swift types (like `Greeting` here) and you can test much of it
locally; keep SwiftUI views thin so CI is the only thing that needs Xcode.
