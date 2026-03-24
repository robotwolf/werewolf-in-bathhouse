import os
import sys

import unreal

SCRIPT_DIR = os.path.dirname(__file__)
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)

from sync_staging_conversation_topics import sync_staging_conversation_topics


NPC_DATA_ROOT = "/Game/WerewolfBH/Data/NPC"
NPC_PROFILE_PATH = f"{NPC_DATA_ROOT}/Profiles"
EXPECTED_NPC_PROFILE_IDS = (
    "Ronin",
    "FirstTimer",
    "FitnessObsessive",
    "FloorManager",
    "OccultScholar",
)


NPC_DEFINITIONS = [
    {
        "asset_name": "DA_NPCProfile_Ronin",
        "npc_id": "Ronin",
        "display_name": "Ronin",
        "summary": "A reckless regular with expensive friends, bad impulse control, and the sort of calm that usually means someone else is about to pay for it.",
        "identity_tags": ["NPC.Role.Guest", "NPC.Role.Suspect"],
        "public_trait_tags": ["NPC.Mood.Calm"],
        "secret_affinity_tags": ["Conversation.Topic.Secret", "Conversation.Topic.Moonrise"],
        "preferred_room_tags": ["Room.Function.Relaxation", "Room.Function.Social", "Room.System.HighRisk"],
        "avoided_room_tags": ["Room.Function.Entry"],
        "baseline_stress": 0.35,
        "stress_tolerance": 0.75,
        "base_suspicion_scalar": 1.35,
        "can_be_werewolf": True,
        "werewolf_activity_tags": ["NPC.Activity.Hide", "NPC.Activity.Observe", "NPC.Activity.WerewolfOverride"],
        "werewolf_reveal_tags": ["Reaction.World.Moonrise", "Conversation.Topic.Secret"],
        "debug_color": unreal.LinearColor(0.82, 0.19, 0.16, 1.0),
        "fear_tuning": {
            "tolerance": 0.62,
            "decay_per_second": 0.10,
            "gain_scalar": 1.10,
            "story_pinned": False,
        },
        "conversation_topics": [
            {"topic": "SocialBanter", "weight": 1.4},
            {"topic": "SecretPlaceholder", "weight": 1.2},
            {"topic": "MoonrisePlaceholder", "weight": 0.8},
            {"topic": "CluePlaceholder", "weight": 0.6},
        ],
        "activities": [
            {"tag": "NPC.Activity.Relax", "preferred_rooms": ["Room.Function.Relaxation", "Room.Function.Social"], "blocked_rooms": [], "weight": 2.6},
            {"tag": "NPC.Activity.Gossip", "preferred_rooms": ["Room.Function.Social", "Room.Function.Changing"], "blocked_rooms": [], "weight": 1.4, "requires_partner": True},
            {"tag": "NPC.Activity.Observe", "preferred_rooms": ["Room.Function.Transition", "Room.System.HighRisk"], "blocked_rooms": [], "weight": 1.9},
            {"tag": "NPC.Activity.Hide", "preferred_rooms": ["Room.System.HighRisk", "Room.Function.Maintenance"], "blocked_rooms": ["Room.Function.Entry"], "weight": 1.1},
            {"tag": "NPC.Activity.WerewolfOverride", "preferred_rooms": ["Room.System.HighRisk", "Room.Function.Maintenance"], "blocked_rooms": [], "weight": 1.7, "werewolf_only": True},
        ],
        "phase_overrides": [
            {"phase": unreal.StagingRunPhase.FIRST_SIGNS, "added": ["NPC.Activity.Observe"], "blocked": [], "mood": ["NPC.Mood.Wary"], "stress": 0.1, "suspicion": 0.1},
            {"phase": unreal.StagingRunPhase.MOONRISE, "added": ["NPC.Activity.Hide", "NPC.Activity.WerewolfOverride"], "blocked": ["NPC.Activity.Gossip"], "mood": ["NPC.Mood.Stressed"], "stress": 0.25, "suspicion": 0.2},
            {"phase": unreal.StagingRunPhase.HUNT, "added": ["NPC.Activity.WerewolfOverride"], "blocked": ["NPC.Activity.Relax"], "mood": ["NPC.Mood.Panicked"], "stress": 0.35, "suspicion": 0.35},
        ],
    },
    {
        "asset_name": "DA_NPCProfile_FirstTimer",
        "npc_id": "FirstTimer",
        "display_name": "The First-Timer",
        "summary": "Too polite to leave, too anxious to relax, and fully convinced everyone else in the building understands rules he has not been taught.",
        "identity_tags": ["NPC.Role.Guest", "NPC.Role.Suspect"],
        "public_trait_tags": ["NPC.Mood.Wary"],
        "secret_affinity_tags": ["Conversation.Topic.Alibi"],
        "preferred_room_tags": ["Room.Function.Entry", "Room.Function.Changing", "Room.Function.Transition"],
        "avoided_room_tags": ["Room.System.HighRisk", "Room.Env.Mechanical", "Room.Access.Staff"],
        "baseline_stress": 0.42,
        "stress_tolerance": 0.35,
        "base_suspicion_scalar": 0.95,
        "can_be_werewolf": False,
        "werewolf_activity_tags": [],
        "werewolf_reveal_tags": [],
        "debug_color": unreal.LinearColor(0.71, 0.85, 1.0, 1.0),
        "fear_tuning": {
            "tolerance": 0.30,
            "decay_per_second": 0.22,
            "gain_scalar": 1.40,
            "story_pinned": False,
        },
        "conversation_topics": [
            {"topic": "AlibiPlaceholder", "weight": 1.6},
            {"topic": "IdleSmallTalk", "weight": 1.1},
            {"topic": "SocialBanter", "weight": 0.7},
        ],
        "activities": [
            {"tag": "NPC.Activity.Wait", "preferred_rooms": ["Room.Function.Entry", "Room.Function.Transition"], "blocked_rooms": ["Room.System.HighRisk"], "weight": 2.8},
            {"tag": "NPC.Activity.Observe", "preferred_rooms": ["Room.Function.Transition", "Room.Function.Changing"], "blocked_rooms": ["Room.System.HighRisk"], "weight": 1.4},
            {"tag": "NPC.Activity.Gossip", "preferred_rooms": ["Room.Function.Entry", "Room.Function.Changing"], "blocked_rooms": ["Room.System.HighRisk"], "weight": 0.8, "requires_partner": True},
        ],
        "phase_overrides": [
            {"phase": unreal.StagingRunPhase.FIRST_SIGNS, "added": ["NPC.Activity.Observe"], "blocked": [], "mood": ["NPC.Mood.Stressed"], "stress": 0.1, "suspicion": 0.05},
            {"phase": unreal.StagingRunPhase.MOONRISE, "added": ["NPC.Activity.Hide"], "blocked": ["NPC.Activity.Gossip"], "mood": ["NPC.Mood.Panicked"], "stress": 0.2, "suspicion": 0.1},
        ],
    },
    {
        "asset_name": "DA_NPCProfile_FitnessObsessive",
        "npc_id": "FitnessObsessive",
        "display_name": "The Fitness Obsessive",
        "summary": "Built like a lecture about discipline and somehow still manages to make stretching look condescending.",
        "identity_tags": ["NPC.Role.Guest", "NPC.Role.Suspect"],
        "public_trait_tags": ["NPC.Mood.Calm"],
        "secret_affinity_tags": ["Conversation.Topic.Gossip"],
        "preferred_room_tags": ["Room.Function.Relaxation", "Room.Env.Cold", "Room.Function.Changing"],
        "avoided_room_tags": ["Room.Env.Mechanical", "Room.Access.Staff"],
        "baseline_stress": 0.18,
        "stress_tolerance": 0.62,
        "base_suspicion_scalar": 1.05,
        "can_be_werewolf": False,
        "werewolf_activity_tags": [],
        "werewolf_reveal_tags": [],
        "debug_color": unreal.LinearColor(0.26, 0.84, 0.68, 1.0),
        "fear_tuning": {
            "tolerance": 0.56,
            "decay_per_second": 0.14,
            "gain_scalar": 0.95,
            "story_pinned": False,
        },
        "conversation_topics": [
            {"topic": "SocialBanter", "weight": 1.3},
            {"topic": "IdleSmallTalk", "weight": 1.0},
            {"topic": "CluePlaceholder", "weight": 0.8},
        ],
        "activities": [
            {"tag": "NPC.Activity.Relax", "preferred_rooms": ["Room.Function.Relaxation", "Room.Env.Cold"], "blocked_rooms": ["Room.Env.Mechanical"], "weight": 2.7},
            {"tag": "NPC.Activity.Observe", "preferred_rooms": ["Room.Function.Relaxation", "Room.Function.Transition"], "blocked_rooms": ["Room.Env.Mechanical"], "weight": 1.2},
            {"tag": "NPC.Activity.Gossip", "preferred_rooms": ["Room.Function.Changing", "Room.Function.Social"], "blocked_rooms": [], "weight": 0.9, "requires_partner": True},
        ],
        "phase_overrides": [
            {"phase": unreal.StagingRunPhase.FIRST_SIGNS, "added": ["NPC.Activity.Observe"], "blocked": [], "mood": ["NPC.Mood.Wary"], "stress": 0.05, "suspicion": 0.05},
            {"phase": unreal.StagingRunPhase.MOONRISE, "added": ["NPC.Activity.Observe"], "blocked": ["NPC.Activity.Gossip"], "mood": ["NPC.Mood.Stressed"], "stress": 0.14, "suspicion": 0.08},
        ],
    },
    {
        "asset_name": "DA_NPCProfile_FloorManager",
        "npc_id": "FloorManager",
        "display_name": "The Floor Manager",
        "summary": "Unreasonably calm in the face of disaster, which is either professionalism or the first line of a disciplinary hearing.",
        "identity_tags": ["NPC.Role.Staff", "NPC.Role.Manager", "NPC.Role.Suspect"],
        "public_trait_tags": ["NPC.Mood.Calm"],
        "secret_affinity_tags": ["Conversation.Topic.Task", "Conversation.Topic.Secret"],
        "preferred_room_tags": ["Room.Function.Entry", "Room.Function.Maintenance", "Room.Function.Transition", "Room.Access.Staff"],
        "avoided_room_tags": ["Room.Env.Cold"],
        "baseline_stress": 0.22,
        "stress_tolerance": 0.82,
        "base_suspicion_scalar": 1.15,
        "can_be_werewolf": False,
        "werewolf_activity_tags": [],
        "werewolf_reveal_tags": [],
        "debug_color": unreal.LinearColor(0.96, 0.77, 0.29, 1.0),
        "fear_tuning": {
            "tolerance": 0.82,
            "decay_per_second": 0.20,
            "gain_scalar": 0.80,
            "story_pinned": True,
        },
        "conversation_topics": [
            {"topic": "TaskPlaceholder", "weight": 1.5},
            {"topic": "SecretPlaceholder", "weight": 1.0},
            {"topic": "IdleSmallTalk", "weight": 0.8},
        ],
        "activities": [
            {"tag": "NPC.Activity.Observe", "preferred_rooms": ["Room.Function.Entry", "Room.Function.Transition"], "blocked_rooms": [], "weight": 2.4},
            {"tag": "NPC.Activity.Clean", "preferred_rooms": ["Room.Function.Maintenance", "Room.Function.Task", "Room.Access.Staff"], "blocked_rooms": [], "weight": 2.2},
            {"tag": "NPC.Activity.Wait", "preferred_rooms": ["Room.Function.Entry"], "blocked_rooms": [], "weight": 0.9},
        ],
        "phase_overrides": [
            {"phase": unreal.StagingRunPhase.FIRST_SIGNS, "added": ["NPC.Activity.Clean"], "blocked": [], "mood": ["NPC.Mood.Wary"], "stress": 0.08, "suspicion": 0.04},
            {"phase": unreal.StagingRunPhase.MOONRISE, "added": ["NPC.Activity.Observe", "NPC.Activity.Clean"], "blocked": ["NPC.Activity.Wait"], "mood": ["NPC.Mood.Stressed"], "stress": 0.16, "suspicion": 0.1},
        ],
    },
    {
        "asset_name": "DA_NPCProfile_OccultScholar",
        "npc_id": "OccultScholar",
        "display_name": "The Occult Scholar",
        "summary": "Claims to be here to unwind, which would land better if his version of unwinding involved even one normal sentence.",
        "identity_tags": ["NPC.Role.Guest", "NPC.Role.Suspect"],
        "public_trait_tags": ["NPC.Mood.Calm"],
        "secret_affinity_tags": ["Conversation.Topic.Secret", "Conversation.Topic.Moonrise"],
        "preferred_room_tags": ["Room.Env.Hot", "Room.Function.Relaxation", "Room.System.ClueDense", "Room.System.HighRisk"],
        "avoided_room_tags": ["Room.Function.Entry"],
        "baseline_stress": 0.16,
        "stress_tolerance": 0.58,
        "base_suspicion_scalar": 1.2,
        "can_be_werewolf": False,
        "werewolf_activity_tags": [],
        "werewolf_reveal_tags": [],
        "debug_color": unreal.LinearColor(0.56, 0.42, 0.95, 1.0),
        "fear_tuning": {
            "tolerance": 0.48,
            "decay_per_second": 0.12,
            "gain_scalar": 1.00,
            "story_pinned": False,
        },
        "conversation_topics": [
            {"topic": "CluePlaceholder", "weight": 1.6},
            {"topic": "SecretPlaceholder", "weight": 1.4},
            {"topic": "MoonrisePlaceholder", "weight": 1.1},
            {"topic": "IdleSmallTalk", "weight": 0.5},
        ],
        "activities": [
            {"tag": "NPC.Activity.Relax", "preferred_rooms": ["Room.Env.Hot", "Room.Function.Relaxation"], "blocked_rooms": [], "weight": 2.1},
            {"tag": "NPC.Activity.Observe", "preferred_rooms": ["Room.System.ClueDense", "Room.System.HighRisk"], "blocked_rooms": ["Room.Function.Entry"], "weight": 1.9},
            {"tag": "NPC.Activity.Gossip", "preferred_rooms": ["Room.Function.Social"], "blocked_rooms": [], "weight": 0.7, "requires_partner": True},
            {"tag": "NPC.Activity.Hide", "preferred_rooms": ["Room.System.HighRisk"], "blocked_rooms": [], "weight": 0.8},
        ],
        "phase_overrides": [
            {"phase": unreal.StagingRunPhase.FIRST_SIGNS, "added": ["NPC.Activity.Observe"], "blocked": [], "mood": ["NPC.Mood.Wary"], "stress": 0.05, "suspicion": 0.05},
            {"phase": unreal.StagingRunPhase.MOONRISE, "added": ["NPC.Activity.Hide", "NPC.Activity.Observe"], "blocked": ["NPC.Activity.Gossip"], "mood": ["NPC.Mood.Stressed"], "stress": 0.16, "suspicion": 0.12},
        ],
    },
]


def log(message: str) -> None:
    unreal.log(f"[sync_staging_npc_profiles] {message}")


def ensure_folder(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def create_data_asset(asset_name: str, package_path: str, asset_class):
    asset_path = f"{package_path}/{asset_name}"
    existing = unreal.load_asset(asset_path)
    if existing:
        return existing

    ensure_folder(package_path)
    factory = unreal.DataAssetFactory()
    configured = False
    for property_name in ("DataAssetClass", "data_asset_class"):
        try:
            factory.set_editor_property(property_name, asset_class)
            configured = True
            break
        except Exception:
            continue

    if not configured:
        raise RuntimeError(f"Could not configure DataAssetFactory for {asset_class}")

    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(asset_name, package_path, asset_class, factory)
    if not asset:
        raise RuntimeError(f"Failed to create data asset: {asset_path}")
    return asset


def save_asset(asset) -> None:
    unreal.EditorAssetLibrary.save_loaded_asset(asset)


def make_tag(tag_name: str):
    return unreal.WerewolfGameplayTagLibrary.make_gameplay_tag_from_name(tag_name, False)


def make_tag_container(tag_names):
    return unreal.WerewolfGameplayTagLibrary.make_gameplay_tag_container_from_names(list(tag_names), False)


def make_text(value: str):
    return unreal.TextLibrary.conv_string_to_text(value)


def set_optional_editor_property(asset, property_name: str, value) -> bool:
    try:
        asset.set_editor_property(property_name, value)
        return True
    except Exception:
        return False


def build_activity_preference(definition):
    preference = unreal.StagingActivityPreference()
    preference.set_editor_property("ActivityTag", make_tag(definition["tag"]))
    preference.set_editor_property("PreferredRoomTags", make_tag_container(definition.get("preferred_rooms", [])))
    preference.set_editor_property("BlockedRoomTags", make_tag_container(definition.get("blocked_rooms", [])))
    preference.set_editor_property("Weight", definition.get("weight", 1.0))
    preference.set_editor_property("bRequiresSocialPartner", definition.get("requires_partner", False))
    preference.set_editor_property("bWerewolfOnly", definition.get("werewolf_only", False))
    return preference


def build_phase_override(definition):
    override = unreal.StagingPhaseActivityModifier()
    override.set_editor_property("Phase", definition["phase"])
    override.set_editor_property("AddedActivityTags", make_tag_container(definition.get("added", [])))
    override.set_editor_property("BlockedActivityTags", make_tag_container(definition.get("blocked", [])))
    override.set_editor_property("AddedMoodTags", make_tag_container(definition.get("mood", [])))
    override.set_editor_property("StressOffset", definition.get("stress", 0.0))
    override.set_editor_property("SuspicionOffset", definition.get("suspicion", 0.0))
    return override


def build_conversation_topic_weight(definition, topic_assets):
    topic_key = definition.get("topic")
    topic_asset = topic_assets.get(topic_key)
    if not topic_asset:
        return None

    weight = unreal.StagingConversationTopicWeight()
    weight.set_editor_property("Topic", topic_asset)
    weight.set_editor_property("Weight", definition.get("weight", 1.0))
    return weight


def sync_profile(definition, topic_assets):
    profile = create_data_asset(definition["asset_name"], NPC_PROFILE_PATH, unreal.StagingNPCProfile)
    profile.set_editor_property("NPCId", definition["npc_id"])
    profile.set_editor_property("DisplayName", make_text(definition["display_name"]))
    profile.set_editor_property("PublicSummary", make_text(definition["summary"]))
    profile.set_editor_property("IdentityTags", make_tag_container(definition.get("identity_tags", [])))
    profile.set_editor_property("PublicTraitTags", make_tag_container(definition.get("public_trait_tags", [])))
    profile.set_editor_property("SecretAffinityTags", make_tag_container(definition.get("secret_affinity_tags", [])))
    profile.set_editor_property("PreferredRoomTags", make_tag_container(definition.get("preferred_room_tags", [])))
    profile.set_editor_property("AvoidedRoomTags", make_tag_container(definition.get("avoided_room_tags", [])))
    profile.set_editor_property("BaselineActivities", [build_activity_preference(item) for item in definition.get("activities", [])])
    profile.set_editor_property("PhaseOverrides", [build_phase_override(item) for item in definition.get("phase_overrides", [])])
    profile.set_editor_property(
        "ConversationTopics",
        [
            topic_weight
            for topic_weight in (
                build_conversation_topic_weight(topic_definition, topic_assets)
                for topic_definition in definition.get("conversation_topics", [])
            )
            if topic_weight
        ],
    )
    profile.set_editor_property("bCanBeWerewolf", definition.get("can_be_werewolf", True))
    profile.set_editor_property("WerewolfActivityTags", make_tag_container(definition.get("werewolf_activity_tags", [])))
    profile.set_editor_property("WerewolfRevealTags", make_tag_container(definition.get("werewolf_reveal_tags", [])))
    profile.set_editor_property("BaselineStress", definition.get("baseline_stress", 0.1))
    profile.set_editor_property("StressTolerance", definition.get("stress_tolerance", 0.5))
    profile.set_editor_property("BaseSuspicionScalar", definition.get("base_suspicion_scalar", 1.0))
    profile.set_editor_property("DebugColor", definition.get("debug_color", unreal.LinearColor.WHITE))

    fear_tuning = definition.get("fear_tuning", {})
    missing_fear_fields = []
    for property_name, value in (
        ("FearTolerance", fear_tuning.get("tolerance", 0.5)),
        ("FearDecayPerSecond", fear_tuning.get("decay_per_second", 0.15)),
        ("FearGainScalar", fear_tuning.get("gain_scalar", 1.0)),
        ("bStoryPinned", fear_tuning.get("story_pinned", False)),
    ):
        if not set_optional_editor_property(profile, property_name, value):
            missing_fear_fields.append(property_name)

    if missing_fear_fields:
        log(
            f"Skipped fear tuning on {definition['asset_name']} until the C++ properties exist: "
            f"{', '.join(missing_fear_fields)}"
        )

    save_asset(profile)
    log(f"Synchronized NPC profile {definition['asset_name']}.")
    return profile


def main():
    ensure_folder(NPC_DATA_ROOT)
    ensure_folder(NPC_PROFILE_PATH)

    topic_assets = sync_staging_conversation_topics()

    for definition in NPC_DEFINITIONS:
        sync_profile(definition, topic_assets)

    synced_ids = [definition["npc_id"] for definition in NPC_DEFINITIONS]
    missing_ids = [profile_id for profile_id in EXPECTED_NPC_PROFILE_IDS if profile_id not in synced_ids]
    extra_ids = [profile_id for profile_id in synced_ids if profile_id not in EXPECTED_NPC_PROFILE_IDS]

    if missing_ids or extra_ids:
        log(
            "Profile coverage mismatch: "
            f"missing={missing_ids if missing_ids else '[]'}, "
            f"extra={extra_ids if extra_ids else '[]'}"
        )
    else:
        log(f"Verified coverage for {len(EXPECTED_NPC_PROFILE_IDS)} Staging NPC profile types.")

    log(f"Synchronized {len(NPC_DEFINITIONS)} Staging NPC profiles.")


if __name__ == "__main__":
    main()
