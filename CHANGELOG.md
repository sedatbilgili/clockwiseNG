# v3.2.10

This release focuses on Mario clockface shooting-star motion polish and autonomous animation scheduling.

- Bumped firmware version to `3.2.10`.
- Added random shooting-star path generation with constrained coordinates (`right-top` to `left-bottom`) and random step count (`5-25`) at `10ms` per step.
- Enforced movement constraints for the new trail path (`endY <= 40`, `endX >= 1`, minimum delta rules between start/end points).
- Improved trail cleanup by restoring sampled underlying scene pixels instead of leaving `SKY_MASK` artifacts.
- Switched shooting-star trigger from manual web button to automatic random scheduling (`10-60s` interval).

This release is a patch-level animation behavior and timing automation update on top of v3.2.9.

# v3.2.9

This release focuses on Stars v2 expansion, configurability, and natural-looking sky distribution.

- Bumped firmware version to `3.2.9`.
- Added `Stars v2` multi-star system with independent lifecycle per star (`1s` fade-in, `30-60s` bright hold + twinkle, `1s` fade-out, `1s` dark hold).
- Added web/API setting `starsCount` (`0-30`) and kept `starsVersion` selection (`v1`/`v2`) for runtime switching.
- Updated v2 star color behavior so each star picks a random target color from `starColors[5]` instead of all-white.
- Enforced spawn/render rules for v2 stars: sky-only placement, minimum object clearance, minimum inter-star spacing, and proper hiding behind foreground objects.
- Improved v2 spawn quality using line-penalty + soft Poisson scoring to reduce artificial linear alignments.
- Added quota-based v2 distribution target: about `%75` in sky strip and `%25` in lower regions (with safe fallback when constraints are tight).

This release is a patch-level visual quality and configurability update on top of v3.2.8.

# v3.2.8

This release focuses on debug logging control cleanup and startup diagnostics.

- Bumped firmware version to `3.2.8`.
- Grouped serial debug output under dedicated build flags (`OTA`, `WIFI`, `TIME`, `GFX`, `WEB_FREEZE`, `HUB75_PINS`) for cleaner runtime control from `platformio.ini`.
- Added new `CW_DEBUG_SYSTEM` group for boot diagnostics.
- Added startup serial diagnostics (`Reboot reason: ...` and `Boot OK`) behind `CW_DEBUG_SYSTEM`.
- Set default debug profile to minimal output (`CW_DEBUG_SYSTEM=1`, others disabled).

This release is a patch-level diagnostics and logging control update on top of v3.2.7.

# v3.2.7

This release is an OTA test bump for manifest and update channel validation.

- Bumped firmware version to `3.2.7`.
- Synced release metadata in manifest and update artifacts for end-to-end OTA verification.

This release is a patch-level validation update on top of v3.2.6.

# v3.2.6

This release is an OTA validation bump for the latest runtime panel and reconnect improvements.

- Bumped firmware version to `3.2.6` for update flow verification.
- Synced manifest metadata and release docs for the new testable update target.

This release is a patch-level validation update on top of v3.2.5.

# v3.2.5

This release focuses on runtime panel configuration, connectivity smoothness, and release metadata consistency.

- Moved panel type selection from compile-time flags to runtime preferences (`panelType`) so a single firmware build can run across all supported panel variants.
- Added panel type setting to API and web settings UI with restart-based apply mode.
- Renamed panel labels to the new codes: `HOP`, `OAP`, `ACP`, `ABP`.
- Updated display runtime panel initialization to read panel wiring from persisted config at startup.
- Changed panel button handling to be runtime-conditional (`OAP`) instead of compile-time conditional.
- Removed compile-time panel type injection from `panel_profile.py` (script now only selects upload target).
- Fixed Wi-Fi reconnect behavior to avoid long blocking waits that caused severe animation slowdowns during disconnect/recovery.
- Improved manifest URL/version/file-name synchronization logic for build-time update artifact generation.

This release is a patch-level runtime configurability and stability update on top of v3.2.4.

# v3.2.4

This release focuses on update distribution packaging, manifest automation, and stability polish for web settings loading.

- Added build-time manifest automation to update firmware metadata (`version`, `publishedAt`, `fileName`, `size`, `sha256`) after each build.
- Added manifest support for primary + fallback firmware URLs and generated a unified `firmware.urls` list for failover-capable updaters.
- Added post-build update artifact packaging that copies the renamed firmware binary and latest `manifest.json` into the `update/` directory.
- Improved web settings startup stability by reducing stack pressure in JSON response/schema handling.
- Kept the Goomba feature set from v3.2.3, including `goombaEnabled` API/UI toggle and 8x8 Goomba rendering behavior.

This release is a patch-level update distribution and stability enhancement on top of v3.2.3.

# v3.2.3

This release focuses on Goomba control, web settings stability, and API/web documentation alignment.

- Added new `goombaEnabled` setting to API and web settings UI so Goomba drawing can be toggled on/off at runtime.
- Updated Mario clockface render flow so Goomba is not drawn at all when disabled, including safe redraw/cleanup when the toggle changes.
- Reduced Goomba render size from `16x16` to `8x8` to match the updated sprite set.
- Improved settings page state parsing for animation toggles (`animationEnabled`, `walkingMario`, `goombaEnabled`) to better handle key/value format differences.
- Fixed web settings load failures by increasing JSON response/schema capacities and reducing stack pressure in JSON response/schema handling.
- Updated API documentation examples and settings reference with `goombaEnabled` usage examples.

This release is a patch-level feature and stability update on top of v3.2.2.

# v3.2.2

This release focuses on OTA status UI polish and progress feedback consistency.

- Replaced alternating OTA update icon frames with the new single `updateIcon` (16x16) asset.
- Added 90-degree OTA icon rotation on each progress bucket step to keep update activity visually clear while using a single icon asset.
- Adjusted OTA progress bar horizontal placement after the width increase so the bar is centered again on the display.
- Reduced browser upload progress status noise by updating the web UI message at `%10` intervals instead of continuously/randomly.

This release is a patch-level OTA UI and UX refinement update on top of v3.2.1.

# v3.2.1

This release focuses on display mode UX polish and OTA progress UI alignment improvements.

- Added a temporary screen mode icon overlay shown for 10 seconds after mode changes, positioned near the top-left corner of the display.
- Added support for the new `onIcon` and `autoIcon` assets (9x9) with `_MASK`-aware transparent rendering behavior.
- Improved rapid mode-change rendering so the mode icon area is cleared before redraw, preventing overlapping pixels when toggled quickly.
- Fixed an intermittent issue where the display could remain dark after switching from `OFF` to `ON` or `AUTO` by synchronizing brightness state transitions more explicitly.
- Improved `AUTO` mode transition behavior after `OFF` by resetting auto-brightness timing/step state for faster re-evaluation.
- Increased OTA progress bar inner width from `40` to `50` pixels.
- Changed OTA screen progress update granularity from `%10` steps to `%2` steps so the bar fill progression better matches the wider progress area.

This release is a patch-level UX and OTA progress visualization update on top of v3.2.0 runtime and OTA architecture improvements.

# v3.2.0

This release focuses on OTA architecture consolidation, non-blocking HTTP OTA flow, and clearer settings apply behavior.

- Unified OTA firmware write/finalize/abort logic behind a shared `OtaRuntime` engine so browser upload OTA and HTTP URL OTA now use the same internal firmware stream pipeline.
- Changed HTTP OTA by URL to a non-blocking staged flow (`BeginRequest`, `Downloading`, `Finalizing`) processed across loop iterations instead of a single blocking transfer call.
- Removed aggressive multi-step restart chaining in OTA restart handling and switched to a single scheduled restart path for more deterministic runtime behavior.
- Improved HTTP OTA request handling by increasing URL capacity and request parsing tolerance in the web action path.
- Added stronger OTA URL handling in web actions with shared validation and safer buffer-based copying.
- Refactored `AppRuntime` loop control into an explicit stage model to make startup, OTA-queued, OTA-in-progress, and normal runtime behavior easier to reason about.
- Added settings apply classification support (`hot` vs `restart`) in the settings schema and settings apply pipeline.
- Extended `POST /api/settings` responses with `changed`, `restartRequired`, and `applyMode` fields to make setting application behavior explicit for clients.
- Updated the settings web UI save feedback to reflect actual backend apply results (hot-applied, restart-required, or no-op) instead of a single generic message.

This release is a feature and maintainability update on top of the v3.1.x OTA and web management improvements.

# v3.1.1

This release focuses on HTTP OTA reliability and progress reporting fixes.

- Fixed HTTP OTA progress calculation so the OTA progress bar no longer appears fully completed too early during download.
- Changed HTTP OTA progress handling to hold at `99%` until firmware finalization succeeds, making the update UI feel more accurate.
- Fixed an HTTP OTA completion issue where the device did not always restart properly after a successful update.
- Improved the end-of-update flow by forcing the final OTA screen refresh before restart.
- Added a small flush-and-delay step before reboot to make the successful HTTP OTA completion path more consistent.

This release is a patch-level stabilization update on top of the new v3.1.0 web and OTA workflow improvements.

# v3.1.0

This release focuses on web firmware management, settings flow cleanup, and maintainability improvements across the web stack.

- Added browser-based OTA firmware upload so a local `.bin` file can be selected from the settings page and installed directly from the device UI.
- Kept HTTP OTA by URL and browser OTA upload side by side, making firmware updates more flexible during development and local use.
- Simplified the settings frontend to use a single canonical API contract instead of legacy field aliases and compatibility mapping logic.
- Removed legacy API compatibility from the active settings flow so the web UI now talks to a cleaner and more explicit JSON model.
- Simplified the `POST /api/settings` contract to accept a single `settings` object format.
- Cleaned up the `GET /api/state` payload by keeping canonical field names only.
- Updated the firmware settings page to use the newer API contract consistently for booleans, numeric values, and OTA actions.
- Centralized settings serialization, validation, and apply behavior behind a shared schema-driven implementation in `CWPreferences`.
- Refactored `AppRuntime` into a clearer orchestration flow by separating startup, OTA, web, render, and display responsibilities into smaller internal steps.
- Split `CWWebServer` responsibilities into separate implementation files for state endpoints, settings endpoints, and action/OTA endpoints, reducing file size and making the web layer easier to maintain.
- Added documentation for the new upload-based OTA endpoint and aligned the local firmware README with the latest web/API behavior.

This release is a feature and maintainability update on top of the v3.0.x API and runtime architecture.

# v3.0.1

This release focuses on web UI responsiveness, API performance, and stability fixes following the v3.0.0 API transition.

- Improved web API response performance, especially for `GET /api/state`.
- Fixed a major slowdown caused by streaming JSON responses directly to the client in small chunks.
- Changed JSON API responses to serialize into a fixed buffer before sending, reducing blocking time in web requests.
- Added route-level timing logs for web requests to help identify slow handlers during development.
- Added internal timing logs for `GET /api/state` to isolate slow response steps more precisely.
- Added short-lived caching for LDR reads to reduce repeated sensor access under frequent polling.
- Changed `sketchUsed`, `sketchFree`, and `flashSize` metrics to be calculated once during web server startup instead of on every `/api/state` request.
- Reduced repeated runtime work inside `/api/state`, improving overall HTTP handling latency.
- Fixed `StatusController` asset migration issues that caused startup screen artifact pixels.
- Restored the full logo bitmap data after the status asset move.
- Fixed `StatusController.cpp` scope and namespace issues introduced during the header-to-`.cpp` refactor.
- Fixed `StatusController` method binding and member access issues in the non-blocking LED/restart flow.
- Fixed JSON response client handling issues in the web server implementation.
- Improved general firmware responsiveness while the settings page is open in the browser.

This release is a stabilization and performance update on top of the new v3.0.x API architecture.


# v3.0.0

This release focuses on firmware architecture cleanup, runtime stability, and a major simplification of the web API.

- The application runtime was reorganized by moving the main firmware flow out of `main.cpp` into a dedicated `AppRuntime` module.
- OTA handling was moved into a separate `OtaRuntime` module.
- Display, brightness, and panel button logic were moved into a separate `DisplayRuntime` module.
- Large header-only implementations were split into `.cpp` files for `CWWebServer`, `WiFiController`, `CWPreferences`, and `StatusController`.
- `StatusController` assets and rendering dependencies were moved out of the header to reduce compile-time and include overhead.
- Wi-Fi handling was redesigned with a clearer internal state flow for connecting, connected, provisioning, recovering, and offline behavior.
- Wi-Fi recovery after disconnect is now more predictable, with timed reconnect attempts and controlled restart behavior after long recovery failure.
- Repeated Wi-Fi checks in the same loop cycle were simplified to avoid unnecessary duplicate state reads.
- The web settings flow was unified so legacy-style field updates and JSON-based settings updates use a shared validation and apply pipeline.
- Preference saving was improved so only changed values are written back, reducing unnecessary flash writes.
- Persistent configuration text fields such as timezone, Wi-Fi SSID, Wi-Fi password, NTP server, and manual POSIX settings were moved from dynamic `String` storage to fixed-size buffers.
- Dynamic `String` usage was reduced in critical runtime paths such as OTA URL handling, settings updates, and web response serialization.
- HTTP OTA URL queuing now uses a fixed buffer instead of dynamic string storage.
- JSON responses are streamed directly to the client instead of being built first in intermediate strings.
- Blocking behavior was reduced by replacing several `delay()`-based flows with non-blocking scheduling.
- LED blink handling is now non-blocking.
- Delayed restart handling is now non-blocking.
- The boot logo wait was changed into a staged non-blocking startup flow.
- All unused BLE-related code was removed from the project.
- Bluetooth cleanup related to the removed BLE path was also removed.
- BLE-related build flags were removed from `platformio.ini`.
- Web API routes were simplified around a new JSON-based model:
  - `GET /api/state`
  - `POST /api/settings`
  - `POST /api/actions`
- Legacy data and action routes such as `/get`, `/set`, `/info`, `/ldr`, `/read`, `/restart`, and `/http-ota` were removed in favor of the new API structure.
- The settings frontend was updated to use the new API endpoints and JSON request bodies.
- Route-level web performance timing was added to help identify slow requests when debug logging is enabled.
- LDR web reads now use short-lived caching to reduce repeated sensor reads under heavy polling.
- Settings update responses were reduced to lightweight success payloads instead of returning full settings data after every save.
- Embedded preference tests were updated to match the new fixed-buffer preference model.

This release contains breaking changes for any tooling, scripts, or clients that relied on the old web endpoints.


# v2.9.0

This release focuses on firmware stability, maintainability, and cleaner runtime behavior.

- The application runtime was reorganized by moving the main firmware flow out of `main.cpp` into a dedicated `AppRuntime` module.
- Large implementations were moved out of headers and into `.cpp` files for `CWWebServer` and `WiFiController`, making the codebase easier to maintain and reducing compile-time coupling.
- Wi-Fi handling was redesigned with a clearer internal state flow for connecting, connected, provisioning, recovering, and offline behavior.
- Wi-Fi recovery after disconnect is now more predictable, with timed reconnect attempts and controlled restart behavior after long recovery failure.
- Repeated Wi-Fi checks in the same loop cycle were simplified to avoid unnecessary duplicate state reads.
- The web settings API was simplified so legacy `/set` updates and JSON `/settings` updates now pass through the same validation and apply pipeline.
- Preference saving was improved so only changed values are written back, reducing unnecessary flash writes.
- Persistent configuration text fields such as timezone, Wi-Fi SSID, Wi-Fi password, NTP server, and manual POSIX settings were moved from dynamic `String` storage to fixed-size buffers.
- Dynamic `String` usage was reduced in critical runtime paths such as OTA URL handling, settings updates, and web response serialization.
- HTTP OTA URL queuing now uses a fixed buffer instead of dynamic string storage.
- JSON responses are now streamed directly to the client instead of first building a temporary response string in memory.
- Blocking behavior was reduced by replacing several `delay()`-based flows with non-blocking scheduling.
- LED blink handling is now non-blocking.
- Delayed restart handling is now non-blocking.
- The boot logo wait was changed into a staged non-blocking startup flow.
- All unused BLE-related code was removed from the project.
- Bluetooth startup cleanup related only to the removed BLE path was also removed.
- BLE-related build flags were removed from `platformio.ini`.
- Embedded preference tests were updated to match the new fixed-buffer preference model.
- This version also includes smaller cleanup, consistency, and internal structure improvements across runtime, settings, and connectivity code.

# 2.8.3:

- A startup rendering bug affecting `PIPE_SPRITE` was fixed so the pipe now appears immediately on boot, including when animations are disabled.

# 2.8.2:

- The OTA workflow was improved for both ArduinoOTA and HTTP OTA paths.
- OTA preparation now more cleanly suspends conflicting runtime services before flashing.
- OTA progress state tracking was improved with more explicit runtime state and diagnostics.
- A dedicated OTA screen was added with update messaging and icon support.
- A progress bar was added to the OTA screen and it advances in `%10` steps.
- The OTA icon animation was redesigned to use fixed frames instead of runtime rotation for lower rendering cost.
- Support for multiple update icons was added, including correct handling of their different dimensions.
- OTA rendering was reworked so progress visualization does not unnecessarily slow down the update process.
- General display update timing and runtime behavior were tuned to better balance rendering, web activity, and OTA stability.

# 2.8.1:

- Timezone handling was made more deterministic by adding built-in POSIX mappings.
- `Asia/Istanbul` now uses a local POSIX timezone mapping instead of relying on remote lookup behavior.
- `UTC` and `Etc/UTC` were also mapped locally for consistency.

# 2.8.0:

- The Wi-Fi startup flow was fixed so a successful captive portal or alternative setup path now counts as a real successful connection.
- Uninitialized runtime flags in the Wi-Fi and web layers were corrected to avoid random restart and state issues.
- Preferences loading was optimized with an in-memory cache to reduce repeated NVS reads.
- The preferences namespace handling was corrected so the configured database name is actually used.
- The web server was refactored away from the old manual socket and HTTP parsing approach to the built-in ESP32 `WebServer` model.
- Several blocking request-handling paths were removed, improving general web responsiveness and reducing freeze risk.
- The settings page script was moved into the firmware and is now served directly by the device instead of an external IP.
- Cache behavior for the settings UI and JSON endpoints was tightened to reduce stale frontend state.
- The settings page pin read flow was fixed so repeated reads work more reliably.
- Repeated `/read` requests were hardened against caching and overlapping XHR issues.
- The active firmware was simplified to the Mario clockface only.
- The unused canvas clockface and its related assets, settings, and helper code were removed.
- Unused canvas-related dependencies were cleaned out of the build.
- Display and clockface lifetime management was moved away from heap allocation to static storage with placement new.
  - This reduced fragmentation risk, especially around OTA and long-running uptime.

# 2.7.4:

- Rendering performance was adjusted to target a lower frame rate for improved stability.
- The display update loop was tuned to approximately `30 FPS`.
- This change was made to reduce rendering load while animations are active and to improve overall responsiveness.

# 2.7.3:

- The virtual scene width was expanded to allow a longer side-scrolling layout.
- A new object (green pipe) was added to the scrolling scene.
- Scene object placement was adjusted to fit the new virtual world layout.
- Background composition was updated so the all elements participate in the scrolling world consistently.

# 2.7.2:

- First version of the fork.
- The Mario-themed scene has been significantly refreshed and made more dynamic.
- A new walking animation for Mario has been added.
- Mario’s classic jump animation at the start of each minute, where he hits the blocks, has been preserved and improved.
- Background scenery elements were also made animated to create a more lively moving scene.
- The overall background layout has been updated with new artwork.
- The scene composition was reworked around a wider virtual world layout.
- Layering between Mario, the background, and foreground elements has been improved.
- Masked and transparent-looking pixels now blend into the scene more correctly.
- Several rendering artifacts and layering issues around Mario were fixed.
- Object rendering issues, including movement-related shifting and edge artifacts, were fixed.
- A new web setting was added to enable or disable Mario’s walking animation.
- ArduinoOTA update functions are now included.
- During OTA updates, unnecessary runtime workload is suspended to keep the update process cleaner.
- OTA failure handling was improved so the device can recover more reliably by restarting.
- The display update pipeline was optimized to reduce rendering load and limit freezes.
- Several redraw and compositing improvements were made to reduce flicker.