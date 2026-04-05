from SCons.Script import Import

Import("env")

PROJECT_CONFIG = env.GetProjectConfig()
selected_profile = PROJECT_CONFIG.get("clockwise", "panel_profile")
if selected_profile is None:
    selected_profile = "oguzali"
selected_profile = selected_profile.strip().lower()

PANEL_PROFILES = {
    "home": {
        "cw_panel_type": "HOME_PANEL",
        "upload_port": "192.168.1.121",
    },
    "oguzali": {
        "cw_panel_type": "OGUZALI_PANEL",
        "upload_port": "192.168.1.133",
    },
    "alican": {
        "cw_panel_type": "ALICAN_PANEL",
        "upload_port": "192.168.1.139",
    },
}

if selected_profile not in PANEL_PROFILES:
    valid = ", ".join(sorted(PANEL_PROFILES))
    raise ValueError(
        f"Unknown panel_profile '{selected_profile}' in [clockwise]. "
        f"Use one of: {valid}"
    )

profile = PANEL_PROFILES[selected_profile]
env.Replace(UPLOAD_PORT=profile["upload_port"])
env.Append(BUILD_FLAGS=[f"-D CW_PANEL_TYPE={profile['cw_panel_type']}"])
