import re

with open("Source/BikeAdventure/Systems/AdvancedBiomePCGSettings.h") as f:
    h_data = f.read()

with open("Source/BikeAdventure/Systems/AdvancedBiomePCGSettings.cpp") as f:
    cpp_data = f.read()

h_arrays = re.findall(r"TArray<TSoftObjectPtr<UStaticMesh>>\s+(\w+);", h_data)
for arr in h_arrays:
    if arr not in cpp_data:
        print(f"Missing in CPP: {arr}")
