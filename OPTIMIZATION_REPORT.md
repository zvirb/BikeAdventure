# Optimization Report - PathNPCSpawner Array Reservation

## Rationale
In `APathNPCSpawner::SpawnNPCsAlongPath()`, the `SpawnedNPCs` array is populated in a loop that runs `NPCCount` times.
Previously, the array was emptied using `SpawnedNPCs.Empty()`, which clears the array but doesn't necessarily reserve memory for the upcoming elements.
As `SpawnedNPCs.Add(Spawned)` is called, the `TArray` might need to reallocate its internal buffer multiple times to accommodate the new elements if the current capacity is exceeded.
Each reallocation involves:
1. Allocating a new, larger memory block.
2. Copying existing elements to the new block.
3. Deallocating the old memory block.

By using `SpawnedNPCs.Empty(NPCCount)`, we explicitly tell the array to reserve space for `NPCCount` elements upfront. This ensures that only one allocation happens (at most) and no reallocations are needed during the loop, resulting in more efficient CPU usage and reduced memory fragmentation.

## Baseline Performance
In the current headless environment, it is impractical to measure the exact microsecond-level improvement of this change as Unreal Engine is not fully installed and the number of NPCs is relatively small (default 5, max 100). However, the theoretical benefits of avoiding $O(n)$ reallocations in a loop are well-established in C++ and Unreal Engine development.

## Implementation Strategy
Change `SpawnedNPCs.Empty();` to `SpawnedNPCs.Empty(NPCCount);` in `Source/BikeAdventure/Gameplay/PathNPCSpawner.cpp`.

## Verification
- Code review of the change.
- Syntax check (as much as possible in the environment).
- Running existing automation tests to ensure no functionality is broken.
