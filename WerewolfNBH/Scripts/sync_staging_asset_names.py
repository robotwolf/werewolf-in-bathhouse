import unreal


ASSET_RENAMES = {
    "/Game/WerewolfBH/Blueprints/NPC/BP_StagehandNPC": "/Game/WerewolfBH/Blueprints/NPC/BP_StagingNPC",
    "/Game/WerewolfBH/Blueprints/NPC/ABP_Quinn_StagehandNPC": "/Game/WerewolfBH/Blueprints/NPC/ABP_Manny_StagingNPC",
    "/Game/WerewolfBH/Data/NPC/Topics/DA_StagehandTopic_IdleSmallTalk": "/Game/WerewolfBH/Data/NPC/Topics/DA_StagingTopic_IdleSmallTalk",
    "/Game/WerewolfBH/Data/NPC/Topics/DA_StagehandTopic_CluePlaceholder": "/Game/WerewolfBH/Data/NPC/Topics/DA_StagingTopic_CluePlaceholder",
    "/Game/WerewolfBH/Data/NPC/Topics/DA_StagehandTopic_SocialBanter": "/Game/WerewolfBH/Data/NPC/Topics/DA_StagingTopic_SocialBanter",
    "/Game/WerewolfBH/Data/NPC/Topics/DA_StagehandTopic_AlibiPlaceholder": "/Game/WerewolfBH/Data/NPC/Topics/DA_StagingTopic_AlibiPlaceholder",
    "/Game/WerewolfBH/Data/NPC/Topics/DA_StagehandTopic_SecretPlaceholder": "/Game/WerewolfBH/Data/NPC/Topics/DA_StagingTopic_SecretPlaceholder",
    "/Game/WerewolfBH/Data/NPC/Topics/DA_StagehandTopic_MoonrisePlaceholder": "/Game/WerewolfBH/Data/NPC/Topics/DA_StagingTopic_MoonrisePlaceholder",
    "/Game/WerewolfBH/Data/NPC/Topics/DA_StagehandTopic_TaskPlaceholder": "/Game/WerewolfBH/Data/NPC/Topics/DA_StagingTopic_TaskPlaceholder",
}


def log(message: str) -> None:
    unreal.log(f"[sync_staging_asset_names] {message}")


def build_rename_data(old_path: str, new_path: str):
    if unreal.EditorAssetLibrary.does_asset_exist(new_path):
        log(f"Skipping rename because destination already exists: {new_path}")
        return None

    asset = unreal.EditorAssetLibrary.load_asset(old_path)
    if not asset:
        log(f"Skipping rename because source was not found: {old_path}")
        return None

    new_package_path, new_name = new_path.rsplit("/", 1)
    return unreal.AssetRenameData(asset=asset, new_package_path=new_package_path, new_name=new_name)


def save_asset_if_present(asset_path: str) -> None:
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.EditorAssetLibrary.save_asset(asset_path, False)


def sync_staging_asset_names() -> None:
    rename_data = []
    for old_path, new_path in ASSET_RENAMES.items():
        data = build_rename_data(old_path, new_path)
        if data:
            rename_data.append(data)

    if rename_data:
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        succeeded = asset_tools.rename_assets(rename_data)
        if not succeeded:
            raise RuntimeError("Asset rename operation reported failure.")

    for new_path in ASSET_RENAMES.values():
        save_asset_if_present(new_path)

    log(f"Verified {len(ASSET_RENAMES)} Staging asset names.")


def main():
    sync_staging_asset_names()


if __name__ == "__main__":
    main()
