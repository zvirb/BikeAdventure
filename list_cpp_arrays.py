import re

with open("Source/BikeAdventure/Systems/AdvancedBiomePCGSettings.cpp") as f:
    cpp_data = f.read()

arr_adds = set(re.findall(r"(\w+)Meshes\.Add\(", cpp_data))
print("Meshes added in CPP:", sorted(list(arr_adds)))
