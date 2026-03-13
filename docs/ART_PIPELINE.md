# BikeAdventure Art Pipeline

This document describes how to generate art assets programmatically using Unreal Engine 5 Python Geometry Scripting and Procedural Material generation.

## Programmatic Generation

As part of the goal to dynamically generate content, there are Python scripts in `scripts/asset_generation/` that construct 3D models and materials dynamically within the editor.

### Prerequisites

- The project must be opened in Unreal Engine 5.4+ with the following plugins enabled:
  - **Python Editor Script Plugin**
  - **Geometry Script**
- To run Python scripts in UE5, ensure you have enabled Python under `Edit > Project Settings > Plugins > Python`.

### Running the Generator Script

1. Open the Unreal Engine Editor for the **BikeAdventure** project.
2. Go to **Window > Output Log**.
3. In the Output Log tab, change the bottom left dropdown from `Cmd` to `Python`.
4. Run the script by entering the full path to it. For example:
   ```python
   import sys
   # Append the path where scripts are if needed, or simply pass the path to execfile
   exec(open("C:/Path/To/Your/Repo/scripts/asset_generation/generate_forest_assets.py").read())
   exec(open("C:/Path/To/Your/Repo/scripts/asset_generation/generate_mountain_assets.py").read())
   exec(open("C:/Path/To/Your/Repo/scripts/asset_generation/generate_urban_assets.py").read())
   exec(open("C:/Path/To/Your/Repo/scripts/asset_generation/generate_countryside_assets.py").read())
   exec(open("C:/Path/To/Your/Repo/scripts/asset_generation/generate_wetlands_assets.py").read())
   exec(open("C:/Path/To/Your/Repo/scripts/asset_generation/generate_desert_assets.py").read())
   exec(open("C:/Path/To/Your/Repo/scripts/asset_generation/generate_beach_assets.py").read())
   exec(open("C:/Path/To/Your/Repo/scripts/asset_generation/generate_bike_assets.py").read())
   ```
   Or if you prefer right-clicking, you can drag and drop `generate_forest_assets.py`, `generate_mountain_assets.py`, `generate_urban_assets.py`, `generate_countryside_assets.py`, `generate_wetlands_assets.py`, `generate_desert_assets.py`, `generate_beach_assets.py`, or `generate_bike_assets.py` into the Content Browser, right click the asset, and select **Run Editor Utility Python Script**.

### Asset Outputs

The scripts will:
- Create `/Game/Art/Models/Forest/`, `/Game/Art/Models/Mountains/`, `/Game/Art/Models/Urban/`, `/Game/Art/Models/Countryside/`, `/Game/Art/Models/Desert/`, `/Game/Art/Models/Beach/`, `/Game/Art/Models/Wetlands/`, `/Game/Art/Models/Character/`, and `/Game/Art/Materials/`.
- Procedurally generate `SM_PineTree` and `SM_ForestRock` for Forest, `SM_MountainRock` and `SM_AlpinePlant` for Mountains, `SM_UrbanBuilding` and `SM_UrbanBench` for Urban, `SM_PalmTree` and `SM_Sandcastle` for Beach, `SM_FarmHouse`, `SM_CountrysideFence`, `SM_Crop`, and `SM_Animal` for Countryside, `SM_WaterBody`, `SM_MarshPlant`, and `SM_WetlandsBridge` for Wetlands, and `SM_Bike` for the Main Bike Character using simple primitive and geometry boolean/noise functions.
- Procedurally construct master materials like `M_StylizedMaster` and `M_StylizedUrban`, and derived material instances (e.g., `MI_MountainRock`, `MI_UrbanBuilding`).
- Assign these material instances to the corresponding static meshes.

These meshes are then automatically linked into the PCG spawning logic for their respective biomes via C++ (e.g., `UMountainPCGSettings` and `UUrbanPCGSettings`).
