> Reference from: https://developer.nvidia.com/gpugems/gpugems/part-vi-beyond-triangles/chapter-38-fast-fluid-dynamics-simulation-gpu

## Navier-Stokes Equation / Incompressible Flow

$$
\frac{\partial u}{\partial t}=-(u\cdot\grad)u-\frac{1}{\rho}\grad p+v\grad^2u+F; \grad\cdot u=0
$$

where $div(x) =\grad\cdot x$ , $curl(x)=\grad\times x$, pressure field $p$, velocity field $u$ , external acceleration $F$

#### Helmholtz Hodge Decomposition

$$
w=u+\grad p
$$

where $u$ is a divergence-free field, $\grad p$ is an irrotational field. Then
$$
\grad \cdot w =\grad \cdot u+\grad^2p=\grad^2p
$$
let $P$ be projection operator. Consider $\grad\cdot u=0$ indicates that $u$ is divergence-free, then
$$
\frac{\partial u}{\partial t}=P\left(-(u\cdot\grad)u+v\grad^2p+F \right)
$$
The equation includes three components, **advection**, **diffusion** and **external force** terms.

#### Advection

Use Gram's algorithm, trace the former grid.
$$
q(x,t+\Delta t)=q(x-u(x,t)\Delta t,t)
$$

#### Diffusion

Forwarding process:
$$
u(x,t+\Delta t)=u(x,t)+v\Delta t \grad^2 u(x,t)
$$
Tracing back:
$$
(I-v\Delta t\grad^2 )\left( u(x,t+\Delta t)\right)= u(x,t)
$$
