# Ensures Jump action exists and Space is mapped in IMC_Combat.
import unreal

ASSET_DIR = "/Game/Variant_Combat/Input"
ACTIONS_DIR = "/Game/Variant_Combat/Input/Actions"
SHARED_JUMP = "/Game/Input/Actions/IA_Jump"
IMC_PATH = "/Game/Variant_Combat/Input/IMC_Combat"


def ensure_dir(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def make_key(name):
    # UE Python accepts Key constructed from the FKey name string
    return unreal.Key(name)


def get_or_create_jump_action():
    for path in (SHARED_JUMP, f"{ACTIONS_DIR}/IA_Jump"):
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            asset = unreal.EditorAssetLibrary.load_asset(path)
            if asset:
                unreal.log(f"Using existing jump action: {path}")
                return asset

    ensure_dir("/Game/Input/Actions")
    ensure_dir(ACTIONS_DIR)
    factory = unreal.InputActionFactory()
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    action = asset_tools.create_asset("IA_Jump", "/Game/Input/Actions", unreal.InputAction, factory)
    if action:
        action.set_editor_property("value_type", unreal.InputActionValueType.BOOLEAN)
        unreal.EditorAssetLibrary.save_asset("/Game/Input/Actions/IA_Jump")
        unreal.log("Created /Game/Input/Actions/IA_Jump")
    return action


def map_space_on_imc(jump_action):
    if not unreal.EditorAssetLibrary.does_asset_exist(IMC_PATH):
        unreal.log_error(f"Missing IMC: {IMC_PATH}")
        return False

    imc = unreal.EditorAssetLibrary.load_asset(IMC_PATH)
    if not imc:
        unreal.log_error("Failed to load IMC_Combat")
        return False

    mappings = list(imc.get_editor_property("mappings") or [])
    cleaned = []
    for m in mappings:
        action = m.get_editor_property("action")
        if action == jump_action:
            continue
        key = m.get_editor_property("key")
        # Drop any existing SpaceBar jump leftovers with a broken action ref
        if str(key) in ("SpaceBar", "Gamepad_FaceButton_Bottom"):
            continue
        cleaned.append(m)

    def make_mapping(key_name):
        m = unreal.EnhancedActionKeyMapping()
        m.set_editor_property("action", jump_action)
        m.set_editor_property("key", make_key(key_name))
        return m

    cleaned.append(make_mapping("SpaceBar"))
    cleaned.append(make_mapping("Gamepad_FaceButton_Bottom"))
    imc.set_editor_property("mappings", cleaned)
    unreal.EditorAssetLibrary.save_asset(IMC_PATH)
    unreal.log("Mapped Space and Gamepad A to Jump on IMC_Combat")
    return True


def main():
    jump = get_or_create_jump_action()
    if not jump:
        unreal.log_error("Could not create/load IA_Jump")
        return
    map_space_on_imc(jump)
    unreal.EditorAssetLibrary.save_directory("/Game/Input")
    unreal.EditorAssetLibrary.save_directory(ASSET_DIR)
    unreal.log("fix_combat_jump_imc.py finished")


if __name__ == "__main__":
    main()
