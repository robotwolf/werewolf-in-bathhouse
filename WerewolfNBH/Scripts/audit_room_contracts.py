import math
import os
import unreal


ROOMS_PATH = "/Game/WerewolfBH/Blueprints/Rooms"
GINNY_ROOMS_PATH = "/Game/WerewolfBH/Data/Ginny/Rooms"
REPORT_PATH = os.path.join(unreal.Paths.project_dir(), "Docs", "RoomContractAudit.md")

ROOM_BOUNDS_LOCATION_TOLERANCE = 1.0
CONNECTOR_FACE_TOLERANCE = 20.0
CONNECTOR_RUN_TOLERANCE = 5.0
CONNECTOR_Z_TOLERANCE = 20.0


def log(message: str) -> None:
    unreal.log(f"[audit_room_contracts] {message}")


def normalize_room_key(name: str) -> str:
    base = name.replace("BP_Room_", "").replace("DA_GinnyRoom_", "")
    return "".join(ch.lower() for ch in base if ch.isalnum())


def get_cdo(blueprint):
    generated = blueprint.generated_class()
    if not generated:
        return None
    return unreal.get_default_object(generated)


def get_parent_name(blueprint) -> str:
    try:
        parent_class = blueprint.get_editor_property("ParentClass")
        if parent_class:
            return parent_class.get_name()
    except Exception:
        pass

    try:
        generated = blueprint.generated_class()
        if generated:
            super_struct = generated.get_super_struct()
            if super_struct:
                return super_struct.get_name()
    except Exception:
        pass

    return "Unknown"


def gather_connectors(blueprint):
    subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    handles = subsystem.k2_gather_subobject_data_for_blueprint(blueprint)
    connectors = []
    for handle in handles:
        data = subsystem.k2_find_subobject_data_from_handle(handle)
        obj = unreal.SubobjectDataBlueprintFunctionLibrary.get_associated_object(data)
        if obj and isinstance(obj, unreal.PrototypeRoomConnectorComponent):
            connectors.append(obj)
    connectors.sort(key=lambda connector: connector.get_name())
    return connectors


def load_ginny_assets():
    ginny_assets = {}
    for asset_path in unreal.EditorAssetLibrary.list_assets(GINNY_ROOMS_PATH, recursive=False, include_folder=False):
        asset_name = asset_path.split("/")[-1].split(".")[0]
        ginny_assets[normalize_room_key(asset_name)] = asset_path
    return ginny_assets


def vector_to_string(vector) -> str:
    return f"({vector.x:.1f}, {vector.y:.1f}, {vector.z:.1f})"


def forward_from_rotator(rotator):
    yaw_radians = math.radians(rotator.yaw)
    pitch_radians = math.radians(rotator.pitch)
    cos_pitch = math.cos(pitch_radians)
    return unreal.Vector(
        math.cos(yaw_radians) * cos_pitch,
        math.sin(yaw_radians) * cos_pitch,
        math.sin(pitch_radians),
    )


def classify_room(blueprint, ginny_asset_map):
    asset_name = blueprint.get_name()
    cdo = get_cdo(blueprint)
    if not cdo:
        return {
            "room": asset_name,
            "parent": get_parent_name(blueprint),
            "ginny_asset": ginny_asset_map.get(normalize_room_key(asset_name), "Missing"),
            "category": "follow-up after hall fix",
            "notes": ["Blueprint has no generated class / CDO."],
            "connector_rows": [],
        }

    room_bounds = cdo.get_editor_property("RoomBoundsBox")
    stock_settings = cdo.get_editor_property("StockAssemblySettings")
    bounds_loc = room_bounds.get_editor_property("RelativeLocation")
    bounds_extent = room_bounds.get_editor_property("BoxExtent")

    bounds_min = unreal.Vector(
        bounds_loc.x - bounds_extent.x,
        bounds_loc.y - bounds_extent.y,
        bounds_loc.z - bounds_extent.z,
    )
    bounds_max = unreal.Vector(
        bounds_loc.x + bounds_extent.x,
        bounds_loc.y + bounds_extent.y,
        bounds_loc.z + bounds_extent.z,
    )

    floor_thickness = max(1.0, float(stock_settings.get_editor_property("FloorThickness")))
    ceiling_thickness = max(0.0, float(stock_settings.get_editor_property("CeilingThickness")))
    floor_top = bounds_min.z + floor_thickness
    ceiling_bottom = bounds_max.z - ceiling_thickness

    notes = []
    bounds_suspect = False
    connector_suspect = False

    if abs(bounds_loc.x) > ROOM_BOUNDS_LOCATION_TOLERANCE or abs(bounds_loc.y) > ROOM_BOUNDS_LOCATION_TOLERANCE:
        bounds_suspect = True
        notes.append(f"RoomBoundsBox is offset in X/Y: loc={vector_to_string(bounds_loc)}")

    if abs(bounds_loc.z - bounds_extent.z) > ROOM_BOUNDS_LOCATION_TOLERANCE:
        bounds_suspect = True
        notes.append(
            f"RoomBoundsBox Z anchor is unusual: loc.z={bounds_loc.z:.1f} extent.z={bounds_extent.z:.1f}"
        )

    connectors = gather_connectors(blueprint)
    connector_rows = []

    if not connectors:
        connector_suspect = True
        notes.append("No connector components found on blueprint.")

    for connector in connectors:
        rel_loc = connector.get_editor_property("RelativeLocation")
        rel_rot = connector.get_editor_property("RelativeRotation")
        forward = forward_from_rotator(rel_rot)

        if abs(forward.x) < 0.001 and abs(forward.y) < 0.001:
            connector_suspect = True
            notes.append(f"{connector.get_name()} has near-zero forward vector.")
            connector_rows.append(
                {
                    "name": connector.get_name(),
                    "face_delta": "n/a",
                    "status": "connector suspect",
                    "note": "Near-zero forward vector",
                }
            )
            continue

        if abs(forward.x) >= abs(forward.y):
            axis = "X"
            normal_sign = 1.0 if forward.x >= 0.0 else -1.0
            expected_face = bounds_loc.x + normal_sign * bounds_extent.x
            actual_face = rel_loc.x
            run_value = rel_loc.y
            run_min = bounds_min.y
            run_max = bounds_max.y
        else:
            axis = "Y"
            normal_sign = 1.0 if forward.y >= 0.0 else -1.0
            expected_face = bounds_loc.y + normal_sign * bounds_extent.y
            actual_face = rel_loc.y
            run_value = rel_loc.x
            run_min = bounds_min.x
            run_max = bounds_max.x

        face_delta = abs(actual_face - expected_face)
        connector_notes = []

        if face_delta > CONNECTOR_FACE_TOLERANCE:
            connector_suspect = True
            connector_notes.append(
                f"{axis}-face delta {face_delta:.1f} (> {CONNECTOR_FACE_TOLERANCE:.1f})"
            )

        if run_value < run_min - CONNECTOR_RUN_TOLERANCE or run_value > run_max + CONNECTOR_RUN_TOLERANCE:
            connector_suspect = True
            connector_notes.append(
                f"Run axis escapes bounds ({run_value:.1f} not in [{run_min:.1f}, {run_max:.1f}])"
            )

        if rel_loc.z < floor_top - CONNECTOR_Z_TOLERANCE or rel_loc.z > ceiling_bottom + CONNECTOR_Z_TOLERANCE:
            connector_suspect = True
            connector_notes.append(
                f"Z {rel_loc.z:.1f} outside opening band [{floor_top:.1f}, {ceiling_bottom:.1f}]"
            )

        connector_rows.append(
            {
                "name": connector.get_name(),
                "face_delta": f"{face_delta:.1f}",
                "status": "connector suspect" if connector_notes else "good",
                "note": "; ".join(connector_notes) if connector_notes else "Aligned to bounds face",
                "axis": axis,
                "forward": vector_to_string(forward),
                "location": vector_to_string(rel_loc),
                "passage_kind": str(connector.get_editor_property("PassageKind")),
                "boundary_kind": str(connector.get_editor_property("BoundaryKind")),
            }
        )

    if connector_suspect:
        category = "connector suspect"
    elif bounds_suspect:
        category = "bounds suspect"
    else:
        category = "good"

    return {
        "room": asset_name,
        "parent": get_parent_name(blueprint),
        "ginny_asset": ginny_asset_map.get(normalize_room_key(asset_name), "Missing"),
        "category": category,
        "notes": notes if notes else ["No obvious connector/bounds drift against the shared RoomBoundsBox contract."],
        "bounds_loc": vector_to_string(bounds_loc),
        "bounds_extent": vector_to_string(bounds_extent),
        "connector_rows": connector_rows,
    }


def build_report(results):
    counts = {
        "good": 0,
        "connector suspect": 0,
        "bounds suspect": 0,
        "follow-up after hall fix": 0,
    }

    for result in results:
        counts[result["category"]] = counts.get(result["category"], 0) + 1

    lines = [
        "# Room Contract Audit",
        "",
        "This report audits the bathhouse room blueprints against the shared `ARoomModuleBase` contract.",
        "",
        "Parent-contract truths checked during this pass:",
        "- placement alignment still snaps rooms by connector pivots in `RoomGenerator.cpp`",
        "- overlap validation still trusts `RoomBoundsBox` via `GetWorldBounds()`",
        "- connector drift or bounds drift on any room can masquerade as a generator bug",
        "",
        "## Summary",
        "",
        f"- good: {counts.get('good', 0)}",
        f"- connector suspect: {counts.get('connector suspect', 0)}",
        f"- bounds suspect: {counts.get('bounds suspect', 0)}",
        f"- follow-up after hall fix: {counts.get('follow-up after hall fix', 0)}",
        "",
        "## Room Table",
        "",
        "| Room | Parent | Ginny Asset | Category | Bounds Loc | Bounds Extent | Notes |",
        "| --- | --- | --- | --- | --- | --- | --- |",
    ]

    for result in results:
        note_text = " / ".join(result["notes"])
        lines.append(
            f"| `{result['room']}` | `{result['parent']}` | `{result['ginny_asset']}` | **{result['category']}** | `{result.get('bounds_loc', 'n/a')}` | `{result.get('bounds_extent', 'n/a')}` | {note_text} |"
        )

    lines.extend([
        "",
        "## Connector Detail",
        "",
    ])

    for result in results:
        lines.append(f"### `{result['room']}`")
        lines.append("")
        lines.append(f"- category: **{result['category']}**")
        for note in result["notes"]:
            lines.append(f"- {note}")
        if not result["connector_rows"]:
            lines.append("- no connector data available")
            lines.append("")
            continue
        lines.append("")
        lines.append("| Connector | Axis | Rel Loc | Forward | Face Delta | Contract | Status | Notes |")
        lines.append("| --- | --- | --- | --- | --- | --- | --- | --- |")
        for row in result["connector_rows"]:
            contract = f"{row.get('passage_kind', 'n/a')} / {row.get('boundary_kind', 'n/a')}"
            lines.append(
                f"| `{row['name']}` | `{row.get('axis', 'n/a')}` | `{row.get('location', 'n/a')}` | `{row.get('forward', 'n/a')}` | `{row['face_delta']}` | `{contract}` | {row['status']} | {row['note']} |"
            )
        lines.append("")

    return "\n".join(lines) + "\n"


def main():
    ginny_asset_map = load_ginny_assets()
    room_blueprint_paths = unreal.EditorAssetLibrary.list_assets(ROOMS_PATH, recursive=False, include_folder=False)
    room_blueprint_paths = sorted(path for path in room_blueprint_paths if path.split("/")[-1].startswith("BP_Room_"))

    results = []
    for asset_path in room_blueprint_paths:
        blueprint = unreal.load_asset(asset_path)
        if not blueprint:
            log(f"Skipping missing blueprint: {asset_path}")
            continue
        results.append(classify_room(blueprint, ginny_asset_map))

    report = build_report(results)
    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        handle.write(report)

    log(f"Wrote audit report: {REPORT_PATH}")
    for result in results:
        log(f"{result['room']}: {result['category']}")


if __name__ == "__main__":
    main()
