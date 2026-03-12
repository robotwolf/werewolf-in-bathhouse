import unreal
modes = [a for a in dir(unreal.BlendMode) if a.startswith('BLEND_')]
path = r"E:\Documents\Projects\werewolf-in-bathhouse\WerewolfNBH\Scripts\BlendModes.txt"
with open(path, "w", encoding="utf-8") as f:
    f.write("\n".join(modes))
