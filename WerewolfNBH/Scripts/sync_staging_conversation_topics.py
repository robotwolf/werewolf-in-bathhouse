import os
import sys

import unreal

SCRIPT_DIR = os.path.dirname(__file__)
if SCRIPT_DIR not in sys.path:
    sys.path.append(SCRIPT_DIR)


NPC_DATA_ROOT = "/Game/WerewolfBH/Data/NPC"
NPC_TOPIC_PATH = f"{NPC_DATA_ROOT}/Topics"


TOPIC_DEFINITIONS = [
    {
        "asset_name": "DA_StagingTopic_IdleSmallTalk",
        "topic_id": "IdleSmallTalk",
        "prompt": "Placeholder idle chatter.",
        "topic_tags": ["Conversation.Topic.Idle", "Conversation.Topic.SmallTalk"],
        "allowed_room_tags": ["Room.Function.Relaxation", "Room.Function.Social", "Room.Function.Transition"],
        "allowed_phase_tags": [],
        "required_speaker_tags": [],
        "blocked_speaker_tags": [],
        "min_stress": 0.0,
        "max_stress": 1.0,
        "min_suspicion": 0.0,
        "max_suspicion": 1.0,
        "neutral_responses": [
            "Bla bla blah? Bla ba Bla Bla!",
            "Just keeping busy. You know how it is.",
        ],
        "pressured_responses": [
            "Yeah, no, I'm fine. Totally fine.",
            "Let's not make this weird.",
        ],
        "werewolf_responses": [
            "Nothing to see here. Move along.",
        ],
    },
    {
        "asset_name": "DA_StagingTopic_CluePlaceholder",
        "topic_id": "CluePlaceholder",
        "prompt": "Clue placeholder text.",
        "topic_tags": ["Conversation.Topic.Clue", "Conversation.Topic.Investigation"],
        "allowed_room_tags": ["Room.System.ClueDense", "Room.System.HighRisk", "Room.Function.Maintenance"],
        "allowed_phase_tags": [],
        "required_speaker_tags": [],
        "blocked_speaker_tags": [],
        "min_stress": 0.0,
        "max_stress": 1.0,
        "min_suspicion": 0.0,
        "max_suspicion": 1.0,
        "neutral_responses": [
            "I found a clue-ish thing. That's the official term now.",
            "There was something here. It was probably important.",
        ],
        "pressured_responses": [
            "I don't know why you're asking me about the clue. I barely know me.",
        ],
        "werewolf_responses": [
            "Clues are for other people.",
        ],
    },
    {
        "asset_name": "DA_StagingTopic_SocialBanter",
        "topic_id": "SocialBanter",
        "prompt": "Basic social banter.",
        "topic_tags": ["Conversation.Topic.Social", "Conversation.Topic.Gossip"],
        "allowed_room_tags": ["Room.Function.Social", "Room.Function.Changing", "Room.Function.Relaxation"],
        "allowed_phase_tags": [],
        "required_speaker_tags": [],
        "blocked_speaker_tags": [],
        "min_stress": 0.0,
        "max_stress": 1.0,
        "min_suspicion": 0.0,
        "max_suspicion": 1.0,
        "neutral_responses": [
            "Bla bla blah? Bla ba Bla Bla!",
            "This is normal conversation, allegedly.",
        ],
        "pressured_responses": [
            "Okay, okay, I hear the panic in your decorative small talk.",
        ],
        "werewolf_responses": [
            "Wouldn't you like to know.",
        ],
    },
    {
        "asset_name": "DA_StagingTopic_AlibiPlaceholder",
        "topic_id": "AlibiPlaceholder",
        "prompt": "Where were you?",
        "topic_tags": ["Conversation.Topic.Alibi"],
        "allowed_room_tags": ["Room.Function.Entry", "Room.Function.Transition", "Room.Access.Staff"],
        "allowed_phase_tags": [],
        "required_speaker_tags": [],
        "blocked_speaker_tags": [],
        "min_stress": 0.0,
        "max_stress": 1.0,
        "min_suspicion": 0.0,
        "max_suspicion": 1.0,
        "neutral_responses": [
            "I was exactly where I said I was. Mostly.",
        ],
        "pressured_responses": [
            "Look, the timeline is not helping either of us right now.",
        ],
        "werewolf_responses": [
            "I do not recall. How convenient for everyone.",
        ],
    },
    {
        "asset_name": "DA_StagingTopic_SecretPlaceholder",
        "topic_id": "SecretPlaceholder",
        "prompt": "Keep it quiet.",
        "topic_tags": ["Conversation.Topic.Secret", "Conversation.Topic.Private"],
        "allowed_room_tags": ["Room.Access.Staff", "Room.System.HighRisk", "Room.Function.Maintenance"],
        "allowed_phase_tags": [],
        "required_speaker_tags": [],
        "blocked_speaker_tags": [],
        "min_stress": 0.0,
        "max_stress": 1.0,
        "min_suspicion": 0.0,
        "max_suspicion": 1.0,
        "neutral_responses": [
            "This stays between us. Probably.",
        ],
        "pressured_responses": [
            "Fine. You win the staring contest. Here's the secret-shaped nothing.",
        ],
        "werewolf_responses": [
            "Secrets are easier when nobody asks.",
        ],
    },
    {
        "asset_name": "DA_StagingTopic_MoonrisePlaceholder",
        "topic_id": "MoonrisePlaceholder",
        "prompt": "The moon is doing that thing again.",
        "topic_tags": ["Conversation.Topic.Moonrise", "Conversation.Topic.Werewolf"],
        "allowed_room_tags": ["Room.System.HighRisk", "Room.Function.Relaxation"],
        "allowed_phase_tags": [
            "Staging.Phase.Moonrise",
            "Staging.Phase.Hunt",
        ],
        "required_speaker_tags": [],
        "blocked_speaker_tags": [],
        "min_stress": 0.0,
        "max_stress": 1.0,
        "min_suspicion": 0.0,
        "max_suspicion": 1.0,
        "neutral_responses": [
            "The moon is loud tonight. That's a sentence, apparently.",
        ],
        "pressured_responses": [
            "If the moon keeps this up, I'm calling a manager.",
        ],
        "werewolf_responses": [
            "No comment. Not a single one.",
        ],
    },
    {
        "asset_name": "DA_StagingTopic_TaskPlaceholder",
        "topic_id": "TaskPlaceholder",
        "prompt": "Still on task.",
        "topic_tags": ["Conversation.Topic.Task", "Conversation.Topic.Work"],
        "allowed_room_tags": ["Room.Function.Maintenance", "Room.Access.Staff", "Room.Function.Entry"],
        "allowed_phase_tags": [],
        "required_speaker_tags": [],
        "blocked_speaker_tags": [],
        "min_stress": 0.0,
        "max_stress": 1.0,
        "min_suspicion": 0.0,
        "max_suspicion": 1.0,
        "neutral_responses": [
            "We're on schedule. Miracles happen.",
        ],
        "pressured_responses": [
            "Yes, I'm aware the place is falling apart in a tasteful way.",
        ],
        "werewolf_responses": [
            "Work first. Panic later.",
        ],
    },
]


def log(message: str) -> None:
    unreal.log(f"[sync_staging_conversation_topics] {message}")


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


def sync_topic(definition):
    topic = create_data_asset(definition["asset_name"], NPC_TOPIC_PATH, unreal.StagingConversationTopic)
    topic.set_editor_property("TopicId", definition["topic_id"])
    topic.set_editor_property("TopicPrompt", make_text(definition["prompt"]))
    topic.set_editor_property("TopicTags", make_tag_container(definition.get("topic_tags", [])))
    topic.set_editor_property("AllowedRoomTags", make_tag_container(definition.get("allowed_room_tags", [])))
    topic.set_editor_property("AllowedPhaseTags", make_tag_container(definition.get("allowed_phase_tags", [])))
    topic.set_editor_property("RequiredSpeakerTags", make_tag_container(definition.get("required_speaker_tags", [])))
    topic.set_editor_property("BlockedSpeakerTags", make_tag_container(definition.get("blocked_speaker_tags", [])))
    topic.set_editor_property("MinStress", definition.get("min_stress", 0.0))
    topic.set_editor_property("MaxStress", definition.get("max_stress", 1.0))
    topic.set_editor_property("MinSuspicion", definition.get("min_suspicion", 0.0))
    topic.set_editor_property("MaxSuspicion", definition.get("max_suspicion", 1.0))
    topic.set_editor_property("bWerewolfOnly", definition.get("b_werewolf_only", False))
    topic.set_editor_property("bBlockWerewolf", definition.get("b_block_werewolf", False))
    topic.set_editor_property("NeutralResponses", [make_text(line) for line in definition.get("neutral_responses", [])])
    topic.set_editor_property("PressuredResponses", [make_text(line) for line in definition.get("pressured_responses", [])])
    topic.set_editor_property("WerewolfResponses", [make_text(line) for line in definition.get("werewolf_responses", [])])
    save_asset(topic)
    log(f"Synchronized Staging topic {definition['asset_name']}.")
    return topic


def sync_staging_conversation_topics():
    ensure_folder(NPC_DATA_ROOT)
    ensure_folder(NPC_TOPIC_PATH)

    topic_assets = {}
    for definition in TOPIC_DEFINITIONS:
        topic_assets[definition["topic_id"]] = sync_topic(definition)

    log(f"Synchronized {len(TOPIC_DEFINITIONS)} Staging conversation topics.")
    return topic_assets


def main():
    sync_staging_conversation_topics()


if __name__ == "__main__":
    main()
