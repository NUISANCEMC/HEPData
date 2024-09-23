# NUISANCE HEPData Conventions

This document corresponds to version 0.1 of the [NUISANCE](https://github.com/NUISANCEMC/nuisance) HEPData Conventions.

This document aims to provide a set of conventions on top of the established [HEPData format specification](http://hepdata-submission.readthedocs.io) that allow NUISANCE to automatically construct predictions for datasets from simulated vectors of neutrino interactions. These conventions should not limit the type of measurements that can be expressed and are meant to cover the majority of existing published data. 

We want to implement the minimum set of specific cases that cover the majority of existing and envisioned measurement types, without the expectation that every single measurement will fit in one of our pre-defined types. For other types of measurements, many of the conventions in this document can still be followed to enable a succinct custom NUISANCE implementation through extending one of these types or compositing existing utilities. See [What To Do If My Measurement Doesn't Fit?](#what-to-do-if-my-measurement-doesn-t-fit).

# Table of Contents

+ [Checklist](#checklist)
+ [HEPData Records](#hepdata-records)
  - [Resource References](#resource-references)
  - [Qualifier Quick Reference](#qualifier-quick-reference)
  - [Variable Qualifier](#variable-qualifier)
  - [Measurement Qualifiers](#measurement-qualifiers)
  - [Composite Measurements](#composite-measurements)
  - [Probe Fluxes](#probe-fluxes)
  - [Errors](#errors)
  - [Smearing](#smearing)
  - [Analysis Snippets](#analysis-snippets)
+ [Building a Submission](#building-a-submission)
+ [Tools and Utilities](#tools-and-utilities)
+ [What To Do If My Measurement Doesn't Fit?](#what-to-do-if-my-measurement-doesn-t-fit)

# Checklist

Below is an at-a-glance checklist for producing compliant HEPData records for most measurements. See the rest of the document for details.

* [✅] _Dependent Variables_ that correspond to a cross section measurement **must** have a _Qualifier_ with key `variable_type`. For most measurements, the most appropriate value is `cross_section_measurement`.
* [✅] For each _Independent Variable_, named `var`, in a table, a _Qualifier_ with the key `var:projectfunc` **must** exist on each measurement _Dependent Variable_, and the value **must** be a valid [Resource Reference](#resource-references) to the name of a projection function in a snippet file. See [Projection and Selection Snippets](#projection-and-selection-snippets).
* [✅] Projection functions _should_ be named according to the convention: `<Experiment>_<MeasurementSpecifier>_<Year>_<ProjectionName>_<INSPIREHEPId>`. This avoids symbol naming collisions when loading many records simultaneously.
* [✅] Each measurement _Dependent Variable_ **must** include a least one `probe_flux`    qualifier. See [Probe Flux](#probe-flux).
* [✅] Each measurement _Dependent Variable_ **must** include a least one `target` qualifier. See [Target](#target)
* [✅] Each measurement _Dependent Variable_ _should_ include one `cross_section_units` qualifier. See [Cross Section Units](#cross-section-units)
* [✅] Measurements that include a covariance estimate **must** include an `errors` qualifier. The value **must** be a valid [Resource Reference](#resource-references) to an errors table. See [Errors](#errors).
* [✅] Measurements presented in some smeared space **must** include a `smearing` qualifier. The value **must** be a valid [Resource Reference](#resource-references) to a smearing table. See [Smearing](#smearing).

# HEPData Records

The top level data structure for a HEPData submission is called a Record. It can be referenced by a unique Id number. This document will not unneccessarily detail the HEPData format as it is authoratatively documented [elsewhere](http://hepdata-submission.readthedocs.io). Records are described by one or more `YAML` files and can also contain others files in a range of formats as _Additional Resources_.

**Table**: A HEPData Table broadly corresponds to a set of binned or unbinned axis (_Independent Variables_) definitions and the corresponding values over those axes (_Dependent Variables_). _Dependent Variables_ are used to store measurements, predictions, error and smearing matrices.

**Binned Independent Variables**: The HEPData table format allows for fully generic hyper-rectangular bins in any number of dimensions. This is generic enough for any measurement that the authors are aware of. If your measurement makes use of non-hyper-rectangular bins, see [What To Do If My Measurement Doesn't Fit?](#what-to-do-if-my-measurement-doesn-t-fit) for ideas.

**Qualifiers**: HEPData _Qualifiers_ are Key-Value pairs attached to _Dependent Variables_ as metadata. These conventions describe a number of _Qualifiers_ that _may_ or **must** be present for a table to be compliant and automatically consumeable by NUISANCE.

**Additional Resources**: Additional files can be added at either the record or the table level.

## Resource References

We define a format for record references as below, where `[]` denote optional components, `<>` denote the reference data itself, and `=` specifies a default value:

```
[<type=hepdata>:][<id>][/<resource[:<qualifier>]>]
```

All parts of the reference are optional. In the absence of a reference `id`, the reference is considered an *intra*-record reference, referring to a resource contained within the same HEPData record. In the absence of a `type`, the reference is considered an *inter*-record reference, referring to a resource contained within another HEPData record. In the absence of a `resource` component, the reference is considered a generic link to another record and not a pointer to a specific resource of that record.

The `type` of a reference is freeform, but outside of the special (and default) `hepdata` type a generic referred resource will not be automatically retrievable. As HEPData uses `INSPIREHEP` id's as a foriegn key for its records, the `inspirehep` type can be used to link to the HEPData record corresponding to a specific INSPIREHEP record. Other useful types might include: `arxiv`, `zenodo`, `doi`, among others. To refer to a specific resource, such as a flux prediction or covariance matrix, the `resource` component should be used. The `qualifier` sub-component is resource specific and is included to enable referring to sub-resources. 

Some specific examples with explanations are given below:

| Example Reference           | Comments |
| --------------------------- | -------- |
| `MyCrossSection`            | Refers to a table named `MyCrossSection` in the current HEPData record. |
| `12345/MyCrossSection`      | Refers to a table named `MyCrossSection` in HEPData record `12345`. |
| `inspirehep:123/MyCrossSection` | Refers to a table named `MyCrossSection` in HEPData record with INSPIREHPE id `123`. |
| `hepdata-sandbox:678910/MyCrossSection` | Refers to a table named `MyCrossSection` in HEPData Sandbox record `678910`. |
| `12345/MyCrossSection:Bkg`  | Refers specifically to the `Bkg` _Dependent Variable_ of table `MyCrossSection` in HEPData record `12345`. |
| `12345/moredata.yaml`       | Refers to an _Additional Resource_ file, named `moredata.yaml` in HEPData record `12345`. |
| `12345/flux.root:flux_numu` | Refers to a specific object (in this case a histogram) named `flux_numu` in the _Additional Resource_ file `flux.root` in HEPData record `12345`. |
| `12345/analysis.cxx:SelF`   | Refers to a specific object (in this case a function) named `SelF` in the _Additional Resource_ file `analysis.cxx` in HEPData record `12345`. |

**HEPData Sandbox**: Because the HEPData REST API differentiates between public and sandboxed records, a separate reference type, `hepdata-sandbox`, must be defined to enable access to records that are in the sandbox. Public records should *never* link to sandboxed records, but sandboxed records may link to either other sandboxed records or public records.

## Qualifier Quick Reference

* Measurement Qualifiers

| Qualifier Key             | Required | Example Value |
| ------------------------- | -------- | ------------- |
| `variable_type`           | Yes      | `cross_section_measurement` |
| `measurement_type`        | No       | `flux_averaged_differential_cross_section` |
| `selectfunc`              | Yes      | `ana.cxx:MicroBooNE_CC0Pi_2019_Selection_123456` |
| `<var>:projectfunc`       | Yes      | `ana.cxx:MicroBooNE_CC0Pi_2019_DeltaPT_123456`, `ana.cxx:MicroBooNE_CCInc_2017_PMu_4321` |
| `cross_section_units`     | No       | `pb\|per_target_nucleon\|per_first_bin_width` |
| `target`                  | Yes      | `CH`, `C[12],H[1]`, `1000180400` |
| `probe_flux`              | Yes      | `123456/MicroBooNE_CC0Pi_2019_flux_numu`, `flux_numu[1],flux_nue[0.9]` |
| `test_statistic`          | No       | `chi2`  |
| `errors`                  | No       | `123456/MicroBooNE_CC0Pi_2019_DeltaPT_covar`  |
| `smearing`                | No       | `123456/MicroBooNE_CC0Pi_2019_DeltaPT_smearing`  |

* Additional Measurement Qualifiers for Composite Measurements

| Qualifier Key             | Required | Example Value |
| ------------------------- | -------- | ------------- |
| `selectfunc[1...]`        | No       | `ana.cxx:MicroBooNE_CC0Pi_2019_Selection_123456` |
| `<var>:projectfunc[1...]` | No       | `ana.cxx:MicroBooNE_CC0Pi_2019_DeltaPT_123456` |
| `target[1...]`            | No       | `CH`, `C[12],H[1]`, `1000180400` |
| `probe_flux[1...]`        | No       | `123456/MicroBooNE_CC0Pi_2019_flux_numu`, `flux_numu[1],flux_nue[0.9]` |
| `sub_measurements`        | No       | `MicroBooNE_CC0Pi_2019_DeltaPTx,MicroBooNE_CC0Pi_2019_DeltaPTy` |

* Flux table qualifiers

| Qualifier Key      | Required | Example Value |
| ------------------ | -------- | ------------- |
| `variable_type`    | Yes      | `probe_flux` |
| `probe_particle`   | Yes      | `numu`, `-14` |
| `bin_content_type` | Yes      | `count`, `count_density` |

* Error table qualifiers

| Qualifier Key   | Required | Example Value |
| --------------- | -------- | ------------- |
| `variable_type` | Yes      | `error_table` |
| `error_type`    | Yes      | `covariance`, `correlation`, `universes` |

* Smearing table qualifiers

| Qualifier Key   | Required | Example Value |
| --------------- | -------- | ------------- |
| `variable_type` | Yes      | `smearing_table` |
| `smearing_type` | Yes      | `smearing_matrix` |


## 1. Variable Qualifier

The `variable_type` qualifier is used to explicitly mark any _Dependent Variable_ that follows this convention. _Dependent Variables_ without the `variable_type` qualifier will generally be ignored by NUISANCE. Values other than those in the below table will generally cause a processing error.

| Value                       | Comments |
| --------------------------- | -------- |
| `cross_section_measurement` | Use this for measurements where a single table, probe flux, and set of selection and projection functions can be used to predict the measurement. This covers most measurements. |
| `composite_cross_section_measurement` | Use this for more complicated measurements that require multiple selections, or multiple separate targets, or multiple fluxes. Also use this to construct a 'meta-measurement' that composits multiple other _sub_-measurements. For more details, see [Composite Measurements](#composite-measurements). |
| `probe_flux`                | Use this to mark a variable as describing the flux of a probe particle. |
| `error_table`               | Use this to mark a variable as describing the error of another measurement. While _Dependent Variables_ can contain bin-by-bin independent errors, a separate table is required to describe a matrix of correlated errors for a measurement. For more details, see [Errors](#errors)|
| `smearing_table`            | Use this to mark a variable as describing the input to a smearing process from generator 'true' space to the measurement space. For more details, see [Smearing](#smearing)|

## 2. Measurement Qualifiers

### 2.1 Measurement Type

For each HEPData Table, one `measurement_type` _Qualifer_ **must** exist to signal how a prediction should be transformed before a comparison can be made

| Value                       | Comments |
| --------------------------- | -------- |
| `flux_averaged_differential_cross_section` | The most common type of published neutrino-scattering measurement. A prediction is made by selecting and projecting simulated neutrino interactions from one or more neutrino species drawn from a neutrino energy spectra, or flux shape. |
| `event_rate`                | Some historic data are presented as simple event rates. For this case, only the shape of simulated predictions can be compared to the data. |
| `ratio`                     | Ratios must often be treated specially as, in general, multiple simulation runs are required to make a fully prediction using either different targets or different probe spectra, or both. Ratio measurements **must** always use the `composite_cross_section_measurement` variable type. |
| `total_cross_section`       | Some historic measurements attempt to unfold the neutrino flux shape and neutrino energy reconstruction effects from observations to make direct measurements of the total scattering cross section as a function of neutrino energy. While this approach has fallen out of favor due to issues with model dependence, there are a number of data sets that are important to preserve. |

This list may be extended in future versions.

### 2.2 Selection and Projection Qualifiers

To make a prediction for a measurement from an MCEG event vector, the corresponding signal definition and observable projections must be defined. We use [ProSelecta](https://github.com/NUISANCEMC/ProSelecta) to codify and execute the selection and projection operators on an event vector. Each _Dependent Variable_ that corresponds to a measurement **must** contain one `selectfunc` _Qualifier_ and one `<var>:projectfunc` _Qualifier_ per corresponding _Independent Variable_. The values of these qualifiers should be valid [Resource References](#resource-references) to ProSelecta functions defined in an [Analysis Snippet](#analysis-snippets).

Selection functions return an integer, which can be used with a composite measurement to select subsamples with a single selection function. For more details, see [Composite Measurements](#composite-measurements). For most selection operators, either a 1 (signal) or a 0 (not signal) should be returned.

### 2.3 Cross Section Units Qualifiers

For the majority of published data, a measurement will take the form of a scattering cross section. There are a number of historic conventions for the units and additional target material scale factors used in published cross section measurements. To avoid clumsy parsing of the HEPData units variable metadata, the explicit units for the measured cross section should also be declared in a fixed form in `cross_section_units` _Qualifier_. The value of the qualifier takes the form of `|`-separated 'flags', at most one from each grouping shown in the table below.

For consistency, we follow the NuHepMC reserved keywords for the Unit and the Target scales defined in [G.C.4](https://arxiv.org/pdf/2310.13211#page=6), but add the `PerTargetNeutron` and `PerTargetProton` options to support existing measurements.

| Value                       | Comments |
| --------------------------- | -------- |
| **Unit** | |
| `cm2`  | A common unit for cross-section measurements that requires us to carry around a power-of-ten scale factor that is dangerously close to the [minimum value](https://en.wikipedia.org/wiki/Single-precision_floating-point_format) representable by single precision IEEE 754 floating point numbers. |
| `1e-38 cm2` | Tries to avoid the 1E-38 factor by including it in the unit definition. |
| `pb` | 1 pb = 1E-36 cm2 |
| `nb`  | 1 nb = 1E-33 cm2 |
| **Target Scaling** | |
| `PerTarget` | Include no explicit additional scaling on the total rate for calculated neutrino--target interactions. This will often correspond to a cross section per elemental nucleus, such as carbon. It also covers simple elemental combinations commonly used for hydrocarbon targets, such as CH, CH2, or CH4. It can also be used for more complex molecular targets. |
| `PerTargetNucleon` | Existing neutrino-scattering simulations often report the cross-section per target nucleon and the neutrino--nucleon interaction is considered the dominant fundamental process. |
| `PerTargetNeutron` | Some data reports the cross section per 'active' nucleon, which for neutrino CCQE interactions with nuclei consists only of the bound neutrons. |
| `PerTargetProton`  | As for `PerTargetNeutron` but for processes that can only occur with target protons. |
| **Density Scaling** | |
| `per_bin_width` | Include no explicit additional scaling on the total rate for calculated neutrino--target interactions. This will often correspond to a cross section per elemental nucleus, such as carbon. It also covers simple elemental ratios commonly used for hydrocarbon targets, such as CH, CH2, or CH4. It can also be used for more complex molecular targets. |

The assumed, or default, value for this qualifier, following the majority of published data, is `cross_section_units=1E-38 cm2|PerTarget|per_bin_width`

### 2.4 Target Specification Qualifiers

Target materials can be specified by a fully complete syntax, or a useful shorthand. Annotated examples below.

| Example Value               | Comments |
| --------------------------- | -------- |
| `C` | A carbon target |
| `1000060120` | A carbon 12 target |
| `CH` | A hydrocarbon target with an average carbon:hydrogen nuclear ratio of 1:1 |
| `1000060120,1000010010` | A hydrocarbon target with a nuclear ratio of 1:1 |
| `1000060120[1],1000010010[1]` | A hydrocarbon target with an average carbon:hydrogen **mass ratio** of 1:1 (equivalent to a nuclear ratio of 1:~12) |
| `CH2` | A hydrocarbon target with an average carbon:hydrogen nuclear ratio of 1:2 |
| `Ar` | An argon 40 target |
| `1000180400` | An argon 40 target |
| `1000180390` | An argon 39 target |

For composite measurments, where multiple, separate targets are needed, additional target specifiers, with an indexed key, _e.g._ `target[1]`, can be specified. For more details, see [Composite Measurements](#composite-measurements).

### 2.5 Recommended Test Statistic Qualifiers

Sometimes a naive Chi2 test statistic is not the most appropriate to use for a given data set, or the errors are encoded with a non-gaussian PDF. Some examples currently handled are given as examples here.

| Value               | Comments |
| --------------------| -------- |
| `chi2` |  |
| `shape_only_chi2` | |
| `shape_plus_norm_chi2` |  |
| `poisson_pdf` | |

### 2.6 Related Table Qualifiers

The remaining measurement _Qualifiers_ all comprise references to other tables that contain related or required information.

| Qualifier Key | Usage |
| ------------- | ------|
| `probe_flux`  | One or more comma-separated [Resource References](#resource-reference) to [Probe Flux](#probe-flux) tables. If multiple references are specified, the cross section from each flux will be weighted by the ratio of the integrals of the referenced flux tables by default. Relative weights for the different components can be forced similarly to the [Target](#2.4-target-specification-qualifiers) specifiers. For example, `flux_numu[1],flux_nue[2]` would produce a combined event rate for the two flux components with the contribution from `flux_nue` scaled up by 2. |
| `errors`      | A simple, single [Resource Reference](#resource-reference) to a table describing the correlated errors for this measurement. For more details, see [Errors](#errors). |
| `smearing`    | A simple, single [Resource Reference](#resource-reference) to a table describing the smearing procedure for this measurment. For more details, see [Smearing](#smearing). |

## 3. Composite Measurements

Composite measurements are both relatively rare and difficult to solve the 'automation' problem for, in general. Instead, we aim to provide some useful coverage for composite measurements that we have encountered, and hopefully leave enough space in the specification for new, powerful composite to fit in the future.

Tables with `variable_type=composite_cross_section_measurement` can specify multiple `selectfunc`, `<var>:projectfunc`, `target`, and `probe_flux` _Qualifiers_ by postfixing the relevant keys with an index, for example, `selectfunc[1]`. These indexes must be sequential, however, `[0]` may be omitted as it is considered implicit in the key `selectfunc`.

The first step of parsing a composite_cross_section_measurement is a check that the number of components is valid. Generally, if only a single instance of one of these _Qualifiers_ is given, it is assumed that that can be used for all components of the composite measurement. If multiple instances are given for any _Qualifier_, then other _Qualifiers_ that also have multiple instances must have the same number. You cannot have a measurement with `selectfunc`, `selectfunc[1]`, `target`, `target[1]`, and `target[2]`.

**Signal Selection Function**: If a single selection function is given for a measurement, it is assumed that the integer returned corresponds to the 1-indexed component measurement that the event should be sifted into, i.e. if the selection function returns a `1`, it will be included in the first component measurement, if it returns a `2`, it will be included in the second component measurement, if it returns a `0`, the event is considered not-signal for all components. If you need events to be included in multiple components of the simultaneous measurement, then you **must** provide a `selectfunc[i]` key for each component, these keys can point to the same ProSelecta function.

**Sub-Measurements**: The `sub_measurements` _Qualifier_ should be used to refer to other measurement tables that this composite measurement combines. This is useful for providing uncertainty estimates for ratios or combinations of other published measurements.

### 3.1 A Target Ratio Example

For a ratio measurement, we want to define exactly 2 component measurements. The first corresponds to the numerator, and the second the denominator of the ratio.

| Qualifier Key             | Value |
| ------------------------- | ------------- |
| `variable_type`           | `composite_cross_section_measurement` |
| `measurement_type`        | `ratio` |
| `selectfunc`              | `MINERvA_CCIncRatio_2016_Select_12345` |
| `Q2:projectfunc`          | `MINERvA_CCIncRatio_2016_Q2_12345` |
| `target`                  | `Pb` |
| `target[1]`               | `CH` |
| `probe_flux`              | `543/MINERVA_Flux_2012:flux_numu` |
| `errors`                  | `MINERvA_CCIncRatio_2016_covar_12345`  |

### 3.2 A Meta-measurement Example

A relatively recent trend, that we hope to see continue, is the publication of 'joint' measurements, that provide correlated error estimates for multiple new, or previous, measurements. To achieve this, we could rebuild the separate datasets into a single new table here with an _Independent Variable_ corresponding to the target material and specify the target as `target=C,O`. However, it is better to re-use the existing data where possible and only provide the minimal additional information we need, in this case, a covariance matrix covering both sub measurements.

| Qualifier Key             | Value |
| ------------------------- | ------------- |
| `variable_type`           | `composite_cross_section_measurement` |
| `measurement_type`        | `flux_averaged_differential_cross_section` |
| `sub_measurements`        | `T2K_CC0Pi_C_2017_12345,T2K_CC0Pi_O_2019_12345` |
| `errors`                  | `T2K_CC0Pi_JointCO_2019_covar_12345`  |

## 4. Probe Fluxes

A probe flux _Dependent Variable_ must have two _Qualifiers_ that specify the probe particle, `probe_particle` and how to interpret the _Dependent Variable_ value, `bin_content_type`. Probe particles can either be specified by a PDG MC particle code or by a human-readable shorthand. The shorthands defined are: `nue[bar]`, `numu[bar]`, `e-`, `e+`, `pi-`, `pi+`, `p`. As the two main neutrino beam groups present flux distributions with different conventions, and correspondingly, the different neutrino MCEGs assume flux histograms using different units conventions, we have decided to require explicitity. The below two valid values for the `bin_content_type` _Qualifier_ should be use to specify whether the flux table should be considered a standard histogram in the flux of probes (`count`) or a PDF-like object (`count_density`), for which we often use histograms in HEP. 

| Value               | Comments |
| --------------------| -------- |
| `count` | The bin value corresponds directly to the flux of probes in units specified on the _Dependent Variable_ header. This will usually have units like `neutrinos /cm^2 /POT`. |
| `count_density` | The bin value corresponds directly to the flux **density** of probes in units specified on the _Dependent Variable_ header. This will usually have units like `neutrinos /cm^2 /50 MeV /POT`. |

## 5. Errors

Covariance and correlation matrices should be provided as tables with two _Independent Variables_ corresponding to the _global_ bin number of the measurement(s) that they cover. It is important to take care when providing covariances for multidimensional measurements that the mapping of, for example the 3 dimensional bin `(i,j,k)` to a global index `g`, is done consistently between the data table and the covariance matrix. These conventions assert that the ordering of bins on the data table should exactly match the ordering of the global index in the corresponding error matrix.

The only _Qualifier_ currently specified for error matrices is `error_type`, and the possible values are shown below.

| Value        | Comments |
| ------------ | -------- |
| `covariance` | The absolute covariance. The error on the corresponding bin can be found by simply square rooting the value. |
| `inverse_covariance` | A pre-inverted absolute covariance. Sometimes error estimation can produce ill-conditioned or difficult to invert matrices and it can be useful for measurements to supply pre-inverted matrices for use in defining test statistics. |
| `fractional_covariance` | A fractional covariance. The **fractional** error on the corresponding bin can be found by simply square rooting the value. |
| `correlation` | A correlation matrix. If a correlation matrix is provided, it will usually be converted back to a covariance matrix by assuming the errors provided on the data tables correspond to the standard error on each bin. In this case, the error component `total`, will be used. If no `total` error component can be found on the corresponding table, this should be considered an error. |
| `universes` | A table with one dependent variable per statistical/systematic universe |

While Error tables should use the same format as data tables, the yaml file for the table can be included as an additional resource on the record or on the corresponding measurement table. The file size limits differ for tables and additional resources and covariance matrices of many-binned measurements can be quite large as an uncompressed yaml file.

## 6. Smearing

Publishing non-true space measurements and smearing procedures is relatively rare, but is becoming more common thanks to the desireable statistical properties of techniques like Wiener SVD unfolding. The only valid _Qualifier_ for a smearing table is `smearing_type=smearing_matrix`. The _Independent Variable_ should be defined as in [Errors](#errors).

While Smearing tables should use the same format as data tables, the yaml file for the table can be included as an additional resource on the record or on the corresponding measurement table. The file size limits differ for tables and additional resources and smearing matrices for many-binned measurements can be quite large as an uncompressed yaml file.

## Analysis Snippets

Analysis Snippets are relatively short C++ source files that contain implementations of event selection and projection operators. These should be written to work with the [ProSelecta](https://github.com/NUISANCEMC/ProSelecta) environment. Generally they are included as a single _Additional Resource_ file per record, which contains all of the functions used by that record.

### Function Naming Conventions

To avoid problems that would be encountered from sequentially loading multiple analysis snippets containing identically named functions, we provide a naming convention for selection and projection functions that should be followed where possible.

| Function Type | Naming Convention | Example  | 
| ------------- | ----------------- | --------- |
| Selection | `<Experiment>_<MeasurementSpecifier>_<Year>_Select_<INSPIREHEPId>` | `int MicroBooNE_CC0Pi_2019_Selection_123456(HepMC3 const &)` |
| Projection | `<Experiment>_<MeasurementSpecifier>_<Year>_<ProjectionName>_<INSPIREHEPId>` | `double MicroBooNE_CC0Pi_2019_DeltaPT_123456(HepMC3 const &)` |

# Building a Data Release

This section contains some useful python examples that you should be able to easily adapt to build a compliant HEPData submission for your data release. It will illustrate building a data release for the T2K on/off axis CC0Pi cross section measurement ([PRD.108.112009](https://doi.org/10.1103/PhysRevD.108.112009)).

We will make extensive use of the useful [hepdata_lib](https://hepdata-lib.readthedocs.io/en/latest/) python module and recommend you do the same. If you have feature requests for the library, please contact us, the authors, or just submit a clear Issue on the [github](https://github.com/HEPData/hepdata_lib). 

## Front Matter

We recommend starting your `ToHepData.py` script with imports and at least the INSPIRE_id for your paper.

```python
#!/usr/bin/env python3

import os, csv
import yaml, ROOT
import requests

from hepdata_lib import Submission, Table, Variable, Uncertainty, RootFileReader

ref = "PRD.108.112009"
INSPIRE_id=2646102
```

In many instances you can make the script download and untar an existing version of the data release for you. This is only helpful if the HEPData release you are building is not the first release of this data.

```python
if not os.path.exists("onoffaxis_data_release/analysis_flux.root"):
  if not os.path.exists("onoffaxis_data_release.tar.gz"):
    # make the initial request
    req = requests.get("https://zenodo.org/records/7768255/files/onoffaxis_data_release.tar.gz?download=1", params={"download": 1})
    # check the response code
    if req.status_code != requests.codes.ok:
      raise RuntimeError("Failed to download data release from: https://zenodo.org/records/7768255/files/onoffaxis_data_release.tar.gz?download=1")
    # write the response to disk
    with open("onoffaxis_data_release.tar.gz", 'wb') as fd:
      for chunk in req.iter_content(chunk_size=128):
        fd.write(chunk)

  # ask a system shell to untar it for you
  os.system("mkdir -p onoffaxis_data_release && tar -zxvf onoffaxis_data_release.tar.gz -C onoffaxis_data_release")
```

## Building a Submission

```python
# instantiate a submission object 
submission = Submission()

# the details of these functions will be detailed below
# -> construct all of the tables constituting the data release
submission.add_table(nd280_analysis())
submission.add_table(ingrid_analysis())
cov,ana = joint_analysis()
submission.add_table(cov)
submission.add_table(ana)
submission.add_table(build_flux_table("ingrid_flux_fine_postfit","flux-onaxis-postfit-fine"))
submission.add_table(build_flux_table("nd280_flux_fine_postfit","flux-offaxis-postfit-fine"))

# add this script to the data release so the steps involved in its creation are transparent
submission.add_additional_resource(description="Python conversion script used to build this submisson. Part of NUISANCE.",
    location="ToHepData.py",
    copy_file=True)

# add the ProSelecta snippet file to the data release
submission.add_additional_resource(description="Selection and projection function examples. Can be executued in the ProSelecta environment v1.0.",
    location="analysis.cxx",
    copy_file=True,
    file_type="ProSelecta")

# add some useful links to the data release
submission.add_link(description="official data release", location="https://doi.org/10.5281/zenodo.7768255")
submission.add_link(description="Use with NUISANCE3", location="https://github.com/NUISANCEMC/nuisance3")
submission.add_link(description="Adheres to the NUISANCE HEPData Conventions", location="https://github.com/NUISANCEMC/HEPData/tree/main")

# build the submission files, ready for upload
submission.create_files(f"submission-{INSPIRE_id}", remove_old=True)
```

## CSV File

We're going to look at parsing the file `nd280_analysis_binning.csv` from the data release in question. It starts like this:

```csv
bin,low_angle,high_angle,low_momentum,high_momentum
1, -1.00, 0.20,    0,  30000
2,  0.20, 0.60,    0,  300
3,  0.20, 0.60,  300,  400
4,  0.20, 0.60,  400,  500
5,  0.20, 0.60,  500,  600
6,  0.20, 0.60,  600,  30000
...
```

We can parse the required bin edge information using the standard library `csv` module:

```python
import csv

cos_thetamu_bins = []
pmu_bins = []

with open("onoffaxis_data_release/nd280_analysis_binning.csv", newline='') as csvfile:
  # by default this class 
  csvreader = csv.DictReader(csvfile)
  for row in csvreader:
    cos_thetamu_bins.append((float(row["low_angle"]), float(row["high_angle"])))
    pmu_bins.append((float(row["low_momentum"])/1E3, float(row["high_momentum"])/1E3))
```

Or we can use the ubiquitous `numpy` module:

```python
import numpy as np

nd280_analysis_binning = np.genfromtxt("onoffaxis_data_release/nd280_analysis_binning.csv",
                                        delimiter=",", skip_header=1)

# take the 2nd and 3rd column as the cos theta bin edges
cos_thetamu_bins = nd280_analysis_binning[:,1:3] 
# and the 4th
pmu_bins = nd280_analysis_binning[:,3:5] / 1E3

```

We then use this to read the binning, data values, and errors from the data release into the relevant `hepdata_lib` objects, as below.

```python
def nd280_analysis():

  nd280_analysis_binning = np.genfromtxt("onoffaxis_data_release/nd280_analysis_binning.csv",
                                        delimiter=",", skip_header=1)

  CosThetaVar = Variable("cos_theta_mu", is_independent=True, is_binned=True, units="")
  CosThetaVar.values = nd280_analysis_binning[:,1:3] 

  PVar = Variable("p_mu", is_independent=True, is_binned=True, units="MeV/c")
  PVar.values = nd280_analysis_binning[:,3:5] / 1E3

  xsec_data_mc = np.genfromtxt("onoffaxis_data_release/xsec_data_mc.csv",
                                          delimiter=",", skip_header=1)

  # the first 58 rows are nd280-only
  nd280_data_mc = xsec_data_mc[:58,...]

  CrossSection = Variable("cross_section", is_independent=False, is_binned=False, 
                          units="$cm${}^{2} c/MeV /Nucleon$")
  CrossSection.values = nd280_data_mc[:,1]

  # qualify the variable type and measurement type
  CrossSection.add_qualifier("variable_type", "cross_section_measurement")
  CrossSection.add_qualifier("measurement_type", "flux_averaged_differential_cross_section")

  # add the selection and projection ProSelecta function reference qualifiers
  CrossSection.add_qualifier("selectfunc", "analysis.cxx:T2K_CC0Pi_onoffaxis_nu_SelectSignal")
  CrossSection.add_qualifier("cos_theta_mu:projectfunc", "analysis.cxx:T2K_CC0Pi_onoffaxis_nu_Project_CosThetaMu")
  CrossSection.add_qualifier("p_mu:projectfunc", "analysis.cxx:T2K_CC0Pi_onoffaxis_nu_Project_PMu")

  # add the target specifier and probe_flux reference qualifiers
  CrossSection.add_qualifier("target", "CH")
  CrossSection.add_qualifier("probe_flux", "flux-offaxis-postfit-fine")

  # if the publication includes predictions, it is often useful to also include
  CrossSectionNEUT = Variable("cross_section_neut-prediction", is_independent=False, 
                              is_binned=False, units="$cm${}^{2} c/MeV /Nucleon$")
  CrossSectionNEUT.values = nd280_data_mc[:,2]

  CrossSectionNEUT.add_qualifier("variable_type", "cross_section_prediction")

  cov_matrix = np.genfromtxt("onoffaxis_data_release/cov_matrix.csv",
                                        delimiter=",")
  nd280_cov_matrix = cov_matrix[:58,:58]

  TotalUncertainty = Uncertainty("total", is_symmetric=True)
  TotalUncertainty.values = np.sqrt(np.diagonal(nd280_cov_matrix))

  CrossSection.add_uncertainty(TotalUncertainty)

  xsTable = Table("cross_section-offaxis")
  xsTable.description = """Extracted ND280 cross section as a function of muon momentum in angle bins compared to the nominal NEUT MC prediction. Note that the final bin extending to 30 GeV=c has been omitted for clarity."""
  xsTable.location = "FIG. 21. in the publication"

  xsTable.add_variable(PVar)
  xsTable.add_variable(CosThetaVar)
  xsTable.add_variable(CrossSection)
  xsTable.add_variable(CrossSectionNEUT)
  xsTable.add_image("fig21.png")

  xsTable.keywords["observables"] = ["D2SIG/DP/DCOSTHETA"]
  xsTable.keywords["reactions"] = ["NUMU C --> MU- P"]
  xsTable.keywords["phrases"] = ["Neutrino CC0Pi", "Cross Section"]

  return xsTable
```

The `ingrid_analysis` method is very similar. Below is a method that builds a `composite_cross_section_measurement` table providing the covariance matrix that spans both the `nd280_analysis` and `ingrid_analysis` data.

```python
def joint_analysis():

  cov_matrix = np.genfromtxt("onoffaxis_data_release/cov_matrix.csv",
                                        delimiter=",")

  # all bin definitions are built as a single array, each bin then has an extent or value in
  # every relevant independent variable, or dimension
  allbins = []
  for j in np.arange(cov_matrix.shape[0]):
    for i in np.arange(cov_matrix.shape[0]):
      allbins.append((i,j))
  allbins = np.array(allbins)

  bin_i = Variable("bin_i", is_independent=True, is_binned=False, units="")
  bin_i.values = allbins[:,0]

  bin_j = Variable("bin_j", is_independent=True, is_binned=False, units="")
  bin_j.values = allbins[:,1]

  Covariance = Variable("covariance", is_independent=False, is_binned=False, units=r"$(cm${}^{2} c/MeV /Nucleon)^{2}$")
  # ravel flattens the array to 1D, row-major by default
  Covariance.values = np.ravel(cov_matrix)

  inv_cov_matrix = np.genfromtxt("onoffaxis_data_release/inv_matrix.csv",
                                        delimiter=",")

  Invcovariance = Variable("inverse_covariance", is_independent=False, is_binned=False, units=r"$(cm${}^{2} c/MeV /Nucleon)^{-2}$")
  Invcovariance.values = np.ravel(inv_cov_matrix)

  Covariance.add_qualifier("variable_type", "error_table")
  Covariance.add_qualifier("error_type", "covariance")

  Invcovariance.add_qualifier("variable_type", "error_table")
  Invcovariance.add_qualifier("error_type", "inverse_covariance")

  covmatTable = Table("covariance-onoffaxis")
  covmatTable.description = """This table contains the covariance and pre-inverted covariance for the joint on/off axis analysis. See the covered measurements for the constituent measurements."""

  covmatTable.add_variable(bin_i)
  covmatTable.add_variable(bin_j)
  covmatTable.add_variable(Covariance)
  covmatTable.add_variable(Invcovariance)

  jointTable = Table("cross_section-onoffaxis")
  CrossSection = Variable("cross_section", is_independent=False)
  CrossSection.add_qualifier("variable_type", "composite_cross_section_measurement")
  CrossSection.add_qualifier("sub_measurements", "cross_section-offaxis,cross_section-onaxis")
  CrossSection.add_qualifier("error", "covariance-onoffaxis:inverse_covariance")
  jointTable.add_variable(CrossSection)

  return (covmatTable,jointTable)
```

## ROOT Histogram

```python
def build_flux_table(hname, tname):
  # instantiate a table object 
  FluxTable = Table(tname)

  # for simple root histograms, we can use the hepdata_lib RootFileReader helper object to
  # convert a histogram into a useful format
  # See https://hepdata-lib.readthedocs.io/en/latest/usage.html#reading-from-root-files
  reader = RootFileReader("onoffaxis_data_release/analysis_flux.root")
  flux_histo = reader.read_hist_1d(hname)

  # define the 'x' axis independent variable
  EnuVar = Variable("e_nu", is_independent=True, is_binned=True, units="GeV")
  EnuVar.values = flux_histo["x_edges"]

  # define the 'y' axis dependent variable
  FluxVar = Variable("flux_nu", is_independent=False, is_binned=False, units="$/cm^{2}/50MeV/10^{21}p.o.t$")
  FluxVar.values = flux_histo["y"]

  # add some all-important qualifiers. qualifiers can only be added to dependent variables
  FluxVar.add_qualifier("variable_type", "probe_flux")
  FluxVar.add_qualifier("probe_particle", "numu")
  FluxVar.add_qualifier("bin_content_type", "count_density")

  # add the variables to the table
  FluxTable.add_variable(EnuVar)
  FluxTable.add_variable(FluxVar)

  # return the table for addition to the submission
  return FluxTable
```

# Tools and Utilities

## `NUISANCEHEPData` Python Module

This repository includes the `NUISANCEHEPData` python module for working with HEPData records following these conventions.

Assuming that the environment variable, `NUISANCEDB` points to a directory to which the current user has write access, a local copy of a record can be downloaded, parsed, and queried with the `NUISHEPDataRecord` class. An example follows:

```python
import os

import NUISANCEHEPData as nhd

os.environ["NUISANCEDB"] = f"{os.environ['home']}/.local/nuisancedb"

record_id = 12345
nhr = nhd.NUISHEPDataRecord(record_id)

print("Cross Section Tables:")
for xsm in nhr.cross_section_measurements:
  print(f"\t name: {xsm.name}")
  for idv in xsm.independent_variables:
    print(f"\t\tselectfunc: {idv.selectfunc}")
    print(f"\t\tprojectfuncs: {idv.projectfuncs}")
    print(f"\t\ttarget: {idv.target}")

print("Additional Resources:")
for ar in nhr.additional_resources:
  print(f"\t{ar}")
```

## `nuis-hepdata` CLI

This repository also contains a CLI tool for querying and populating a local database of HEPData records called `nuis-hepdata`. It is built on the `NUISANCEHEPData` python module but offers shell scripting capabilities for record database management. Full documentation can be obtained by running `./nuis-hepdata help` within this repository, but some example usage is shown below.

### Fetch a Record

This example will use a hepdata-sandbox record, but the examples generalise to public hepdata records identified by their HEPData id or their INSPIRHEP id.

Firstly we will ensure that we have a local copy of the record of interest by running the `get-local-path` command. In general all commands will transparently trigger a record fetch from HEPData.net if a local copy doesn't already exist.

```bash
$ ./nuis-hepdata --nuisancedb ./database get-local-path hepdata-sandbox:1713531371
./database/hepdata-sandbox/1713531371/HEPData-1713531371-v1
```

We can see what is in the record directory:

```bash
$ ls ./database/hepdata-sandbox/1713531371/HEPData-1713531371-v1
ToHepData.py                     cross_section-offaxis.yaml       fig21.png                         flux-offaxis-nominal-fine.yaml    
flux-onaxis-nominal-coarse.yaml  flux-onaxis-postfit-fine.yaml    thumb_fig22.png                   analysis.cxx               
cross_section-onaxis.yaml        fig22.png                        flux-offaxis-postfit-coarse.yaml  flux-onaxis-nominal-fine.yaml
submission.yaml                  covariance-onoffaxis.yaml        cross_section-onoffaxis.yaml      flux-offaxis-nominal-coarse.yaml  
flux-offaxis-postfit-fine.yaml   flux-onaxis-postfit-coarse.yaml  thumb_fig21.png
```

If we want to follow the remote request logic we can add a `--debug` option, as below:

```bash
$ ./nuis-hepdata --nuisancedb ./database --debug get-local-path hepdata-sandbox:1713531371
INFO:HepDataRefResolver:ResolveReferenceIdentifiers(hepdata_reference=hepdata-sandbox:1713531371,context={}):
    -> {'reftype': 'hepdata-sandbox', 'recordid': '1713531371', 'recordvers': '', 'resourcename': '', 'qualifier': ''}
  
INFO:HepDataRefResolver:ResolveHepDataReference(root_dir=./database/hepdata-sandbox,refcomps={'reftype': 'hepdata-sandbox', 'recordid': '1713531371', 'recordvers': '', 'resourcename': '', 'qualifier': ''}):
      local_record_path = ./database/hepdata-sandbox/1713531371/HEPData-1713531371-v (exists: True)
      local_resource_path = ./database/hepdata-sandbox/1713531371/HEPData-1713531371-v (exists: True)

INFO:HepDataRefResolver:ResolveHepDataReference(root_dir=./database/hepdata-sandbox,refcomps={'reftype': 'hepdata-sandbox', 'recordid': '1713531371', 'recordvers': '', 'resourcename': '', 'qualifier': ''}):
      Need to fetch record
  
INFO:HepDataRefResolver:ResolveHepDataReference(root_dir=./database/hepdata-sandbox,refcomps={'reftype': 'hepdata-sandbox', 'recordid': '1713531371', 'recordvers': '', 'resourcename': '', 'qualifier': ''}):
    requests.get(record_URL=https://www.hepdata.net/record/sandbox/1713531371) -> 200

./database/hepdata-sandbox/1713531371/HEPData-1713531371-v1
```

### A Note on Record Versions

As all records include a version qualifier that is often omitted as it is usually '1'. Record references without the version qualifier will always trigger a remote check to see if a later version of the record is available. This request can be elided by qualifying the reference with the version number that you know you have a local copy of. See the difference between the two below requests.

```bash
$ ./nuis-hepdata --nuisancedb ./database --debug get-local-path hepdata-sandbox:1713531371v1
INFO:HepDataRefResolver:ResolveReferenceIdentifiers(hepdata_reference=hepdata-sandbox:1713531371v1,context={}):
    -> {'reftype': 'hepdata-sandbox', 'recordid': '1713531371', 'recordvers': '1', 'resourcename': '', 'qualifier': ''}
  
INFO:HepDataRefResolver:ResolveHepDataReference(root_dir=./database/hepdata-sandbox,refcomps={'reftype': 'hepdata-sandbox', 'recordid': '1713531371', 'recordvers': '1', 'resourcename': '', 'qualifier': ''}):
      local_record_path = ./database/hepdata-sandbox/1713531371/HEPData-1713531371-v1 (exists: True)
      local_resource_path = ./database/hepdata-sandbox/1713531371/HEPData-1713531371-v1 (exists: True)

INFO:HepDataRefResolver:ResolveHepDataReference(root_dir=./database/hepdata-sandbox,refcomps={'reftype': 'hepdata-sandbox', 'recordid': '1713531371', 'recordvers': '1', 'resourcename': '', 'qualifier': ''}):
      Returning local path
  
./database/hepdata-sandbox/1713531371/HEPData-1713531371-v1

$ ./nuis-hepdata --nuisancedb ./database --debug get-local-path hepdata-sandbox:1713531371
INFO:HepDataRefResolver:ResolveReferenceIdentifiers(hepdata_reference=hepdata-sandbox:1713531371,context={}):
    -> {'reftype': 'hepdata-sandbox', 'recordid': '1713531371', 'recordvers': '', 'resourcename': '', 'qualifier': ''}
  
INFO:HepDataRefResolver:ResolveHepDataReference(root_dir=./database/hepdata-sandbox,refcomps={'reftype': 'hepdata-sandbox', 'recordid': '1713531371', 'recordvers': '', 'resourcename': '', 'qualifier': ''}):
      local_record_path = ./database/hepdata-sandbox/1713531371/HEPData-1713531371-v (exists: True)
      local_resource_path = ./database/hepdata-sandbox/1713531371/HEPData-1713531371-v (exists: True)

INFO:HepDataRefResolver:ResolveHepDataReference(root_dir=./database/hepdata-sandbox,refcomps={'reftype': 'hepdata-sandbox', 'recordid': '1713531371', 'recordvers': '', 'resourcename': '', 'qualifier': ''}):
      Need to fetch record
  
INFO:HepDataRefResolver:ResolveHepDataReference(root_dir=./database/hepdata-sandbox,refcomps={'reftype': 'hepdata-sandbox', 'recordid': '1713531371', 'recordvers': '', 'resourcename': '', 'qualifier': ''}):
    requests.get(record_URL=https://www.hepdata.net/record/sandbox/1713531371) -> 200

./database/hepdata-sandbox/1713531371/HEPData-1713531371-v1
```

The second, unqualified attempt has to check the record metadata to ensure that the latest version is the one that we have a local copy of, this (sometimes unneccessary) round trip to the server takes ~ 1 s.

### Querying a Record

The first bit of information we will usually want from a record is what cross-section measurements are contained within it:

```bash
$ ./nuis-hepdata --nuisancedb ./database get-cross-section-measurements hepdata-sandbox:1713531371v1
cross_section-offaxis
cross_section-onaxis
cross_section-onoffaxis
```

This record contains three measurements that we might want to compare to. We probably want to know the independent and dependent variables defined by each measurement, we can request those. Note that now the measurement table name must be included in the reference:

```bash
$ ./nuis-hepdata --nuisancedb ./database get-independent-vars hepdata-sandbox:1713531371v1/cross_section-onaxis
p_mu
cos_theta_mu

$ ./nuis-hepdata --nuisancedb ./database get-dependent-vars hepdata-sandbox:1713531371v1/cross_section-onaxis
cross_section
```

A lot of useful metadata is stored in the qualifiers of dependent variables, we can also query those:

```bash
$ ./nuis-hepdata --nuisancedb ./database get-qualifiers hepdata-sandbox:1713531371v1/cross_section-onaxis
selectfunc:analysis.cxx:T2K_CC0Pi_onoffaxis_nu_SelectSignal
cos_theta_mu:projectfunc:analysis.cxx:T2K_CC0Pi_onoffaxis_nu_Project_CosThetaMu
p_mu:projectfunc:analysis.cxx:T2K_CC0Pi_onoffaxis_nu_Project_PMu
target:CH
probe_flux:flux-onaxis-postfit-fine
variable_type:cross_section_measurement
pretty_name:$p_{\mu}$
```

If we want to get the value of a qualifier that we know exists, we can do that too:

```bash
$ ./nuis-hepdata --nuisancedb ./database get-qualifiers hepdata-sandbox:1713531371v1/cross_section-onaxis selectfunc
analysis.cxx:T2K_CC0Pi_onoffaxis_nu_SelectSignal
```

### Following Qualifier References

It is often useful to be able to treat the value of a qualifier as a record reference itself and resolve a local path for it,
for example when wanting to concretize the probe flux to use in making a measurement prediction. This can be achieved with the
`dereference-to-local-path` command, as demonstrated below: 

```bash
$ ./nuis-hepdata --nuisancedb ./database dereference-to-local-path hepdata-sandbox:1713531371v1/cross_section-onaxis probe_flux
./database/hepdata-sandbox/1713531371/HEPData-1713531371-v1/flux-onaxis-postfit-fine.yaml
```

Some qualifiers can contain a comma-separated-list of references, these will each separately be resolved to a local path.

```bash
$ ./nuis-hepdata --nuisancedb ./database dereference-to-local-path hepdata-sandbox:1713531371v1/cross_section-onoffaxis sub_measurements
./database/hepdata-sandbox/1713531371/HEPData-1713531371-v1/cross_section-offaxis.yaml
./database/hepdata-sandbox/1713531371/HEPData-1713531371-v1/cross_section-onaxis.yaml
```

# What To Do If My Measurement Doesn't Fit?

Weep profusely. To Write... 

In he meantime, reach out to [nuisance-owner\@projects.hepforge.org](mailto:nuisance-owner@projects.hepforge.org) or ask on [nuisance-xsec.slack.com](nuisance-xsec.slack.com).
