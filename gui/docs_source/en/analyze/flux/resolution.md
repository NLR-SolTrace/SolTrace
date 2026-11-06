---
title: "Flux Map Resolution"
---

Flux grid resolution controls the number of flux bins and the exported texture size. Choose a square resolution from 128 × 128 through 4096 × 4096. Higher values preserve more spatial detail, but they can reveal more ray-sampling noise, take longer to compute, and use more memory.

Smoothing reduces ray-sampling noise without lowering the grid resolution. **None** preserves the unsmoothed bin values. **Light**, **Medium**, and **Strong** apply increasingly wide Gaussian smoothing in flux-grid bins. Energy and physical surface area are smoothed together, and the total plotted power is preserved.

Surface detail controls how finely curved or analytic geometry is approximated when ray hits are projected and when physical pixel areas are calculated. It does not set the flux-map bin size. Higher values can improve the accuracy of strongly curved surfaces, but they also increase computation time.

Show Triangle Grid overlays the generated surface mesh for diagnostic purposes. The displayed flux is accumulated in image space and should not follow these triangle edges.
