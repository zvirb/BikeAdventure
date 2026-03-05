## 2024-05-24 - Performance Optimization
**Learning:** Replaced `FVector::Dist` with `FVector::DistSquared` when comparing distances to constants or calculating culling bounds. This optimization removes expensive `sqrt()` operations in inner loops without sacrificing correctness.
**Action:** Always favor squared distance calculation for distance threshold comparisons.
