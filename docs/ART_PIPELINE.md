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
   exec(open("C:/Path/To/Your/Repo/scripts/asset_generation/generate_mountain_assets.py").read())
   ```
   Or if you prefer right-clicking, you can drag and drop `generate_mountain_assets.py` into the Content Browser, right click the asset, and select **Run Editor Utility Python Script**.

### Asset Outputs

The script will:
- Create `/Game/Art/Models/Mountains/` and `/Game/Art/Materials/`.
- Procedurally generate `SM_MountainRock` and `SM_AlpinePlant` using simple primitive and geometry boolean/noise functions.
- Procedurally construct a master material `M_StylizedMaster` and derived material instances (`MI_MountainRock`, `MI_AlpinePlant`).
- Assign these material instances to the corresponding static meshes.

These meshes are then automatically linked into the PCG spawning logic for the Mountains biome via C++ in `UMountainPCGSettings`.
