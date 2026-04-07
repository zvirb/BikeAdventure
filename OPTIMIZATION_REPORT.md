# Performance Optimization Report: Consolidated Component Registration

## 💡 What
The optimization consolidates multiple calls to `AActor::GetComponents<T>` into a single call to `AActor::GetComponents(TArray<UActorComponent*>& OutComponents)`. It then iterates over the retrieved components once, using `Cast<T>` to identify and register both `UStaticMeshComponent` and `UNiagaraComponent` for optimization tracking.

## 🎯 Why
Prior to this change, `UAutoOptimizationComponent::RegisterWithOptimizationSystem` would traverse the actor's component list twice: once to find all static meshes and again to find all particle systems. In actors with many components, this leads to redundant CPU cycles and multiple temporary array allocations.

## 🛠️ Implementation Strategy
- Replaced separate `GetComponents<UStaticMeshComponent>` and `GetComponents<UNiagaraComponent>` calls.
- Introduced a single pass loop over all components.
- Used safe casting to identify target component types.
- Maintained existing registration logic for both component types.

## 📊 Measured Improvement (Theoretical)
While a live profiling session was not possible in the current headless environment, the theoretical benefits include:
- **Reduced Time Complexity:** $O(2N)$ reduced to $O(N)$, where $N$ is the number of components on the actor.
- **Reduced Memory Overhead:** One `TArray` allocation instead of two.
- **Improved Cache Locality:** Single traversal of the component list is more cache-friendly.

These improvements are particularly valuable during level loading or when spawning many procedurally generated actors with `UAutoOptimizationComponent` enabled.
