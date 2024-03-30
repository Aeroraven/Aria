> Main references from: 
>
> https://pbr-book.org/4ed/Volume_Scattering/
>
> https://pbr-book.org/4ed/Light_Transport_II_Volume_Rendering



### 0. Basics

**Monte Carlo Integration (Importance Sampling)**: 
$$
\int_a^bf(x)dx=\lim_{N\to\infty}\frac{1}{N}\sum_{i=0}^N\frac{f(X_i)}{p(X_i)}
$$

### 1. Main Attributes in Scattering Process

- **Absorption:** probability density that light is absorbed per unit distance traveled in the medium
  $$
  dL_o(p,w)=-\sigma_aL_i(p,-w)dt
  $$

- **Emission**
  $$
  dL_o(p,w)=\sigma_aL_e(p,w)dt
  $$

- **Scattering**
  $$
  dL_o(p,w)=-\sigma_sL_i(p,-w)dt
  $$

- **Attenuation**: the total reduction in radiance
  $$
  dL_o(p,w)=-(\sigma_s+\sigma_a)L_i(p,-w)dt=-\sigma_tL_i(p,-w)dt
  $$

- **Albedo**: scattering probability in each event

- **Mean Free Path**: average traveling distance of ray before interacting with the particle
  $$
  \rho(p,w)=\frac{\sigma_s(p,w)}{\sigma_t(p,w)};d(p,w)=\frac{1}{\sigma_t(p,w)}
  $$

- **Phase Function**: angular distribution of ray at scattering, subject to normalization constraint:
  $$
  \int_{S^2} P(w_i,w_o)dw_o=1
  $$
  

​	the source term = self emission + scattered light (as input)
$$
L_s(p,w)=\frac{\sigma_a(p,w)}{\sigma_t(p,w)}L_e(p,w)+\frac{\sigma_s(p,w)}{\sigma_t(p,w)}\int_{S^2}P(p,w_i,w)L_i(p,w_i)dw_i
$$

### 2. Transmittance

- **Beam Transmittance**: fraction of radiance travels between two points
  $$
  \frac{dL_o(p+t,w)}{dt}=-\sigma_t(p+t,w)L_i(p+t,-w)
  $$

  $$
  \int_{L(p,w)}^{L(p+d,w)}\frac{dL(p+t,w)}{L(p+t,w)} = \int_0^d-\sigma_t(p+t,w)dt
  $$

  $$
  L(p+d,w)=L(p,w)\exp\left( \int_0^d-\sigma_t(p+t,w)dt\right)
  $$

  then, the beam transmittance:
  $$
  T(p\to p+d) = \exp\left( \int_0^d-\sigma_t(p+t,w)dt\right)
  $$

- ###### **Optical Thickness**: 

$$
\tau(p\to p+d)=\int_0^d-\sigma_t(p+t,w)dt
$$

for homogeneous medium
$$
\tau(p\to p+d)=\sigma_td
$$

### 3. Phase Function

- **Henyey-Greenstein Phase Function**:
  $$
  P_{HG}(\cos\theta)=\frac{1}{4\pi}\frac{1-g^2}{\left(1+g^2+2g\cos\theta\right)^{3/2}}
  $$
  with $g$ as asymmetry parameter

### 4. The Equation of Transfer

- **The Equation of Transfer**: 

$$
L_i(p,w)=T(s\to p)L_o(s,-w)+\int_0^sT(t\to p)\sigma_t(t,-w)L_s(t,-w)dt
$$

where $T$ is beam transmittance, $L_s$ is source term, $\sigma_t$ is total attenuation
