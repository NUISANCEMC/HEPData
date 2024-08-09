# NUISANCE HEPData Conventions

This document corresponds to version 0.1 of the [NUISANCE](https://github.com/NUISANCEMC/nuisance) HEPData Conventions.

This document aims to provide a set of conventions on top of the established [HEPData format specification](http://hepdata-submission.readthedocs.io) that allow NUISANCE to automatically construct predictions for datasets from simulated vectors of neutrino interactions. These conventions should not limit the type of measurements that can be expressed and are meant to cover the majority of existing published data. 

We want to implement the minimum set of specific cases that cover the majority of existing and envisioned measurement types, without the expectation that every single measurement will fit in one of our pre-defined types. For other types of measurements, many of the conventions in this document can still be followed to enable a succinct custom NUISANCE implementation through extending one of these types or compositing existing utilities. See [What To Do If My Measurement Doesn't Fit?](#what-to-do-if-my-measurement-doesn-t-fit).

# Table of Contents

+ [Checklist](#checklist)
+ [HEPData Records](#hepdata-records)
  - [Tables](#tables)
    * [Qualifiers](#qualifiers)
    * [Types](#types)
    * [Independent Variables](#independent-variables)
    * [Dependent Variables](#dependent-variables)
    * [Formats](#formats)
  - [Additional Resources](#additional-resources)
    * [Projection and Selection Snippets](#projection-and-selection-snippets)
    * [Data Release Conversion Scripts](#data-release-conversion-scripts)
  - [Resource References](#resource-references)
    - [Intra-record](#intra-record)
    - [Inter-record](#inter-record)
    - [INSPIRE](#inspire)
- [Publications](#publications)
  - [Datasets](#datasets)
    - [Selections](#selections)
    - [Projections](#projections)
    - [Multi-dimensional Data](#multi-dimensional-data)
    - [Errors](#errors)
    - [Test Statistics](#test-statistics)
  - [Flux Predictions](#flux-predictions)
    * [Neutrino Energy Cuts](#neutrino-energy-cuts)
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
| `error_type`    | Yes      | `covariance`, `correlation` |

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

| Value                       | Comments |
| --------------------------- | -------- |
| **Prefix Factor** | |
| `cm2`  | A common unit for cross-section measurements that requires us to carry around a power-of-ten scale factor that is dangerously close to the [minimum value](https://en.wikipedia.org/wiki/Single-precision_floating-point_format) representable by single precision IEEE 754 floating point numbers. |
| `1E-38cm2` | Tries to avoid the 1E-38 factor by including it in the unit definition. |
| `pb` | 1 pb = 1E-36 cm2 |
| `nb`  | 1 pb = 1E-33 cm2 |
| **Target Scaling** | |
| `per_target` | Include no explicit additional scaling on the total rate for calculated neutrino--target interactions. This will often correspond to a cross section per elemental nucleus, such as carbon. It also covers simple elemental ratios commonly used for hydrocarbon targets, such as CH, CH2, or CH4. It can also be used for more complex molecular targets. |
| `per_nucleon` | Existing neutrino-scattering simulations often report the cross-section per target nucleon and the neutrino--nucleon interaction is considered the dominant fundamental process. |
| `per_neutron` | Some data reports the cross section per 'active' nucleon, which for neutrino CCQE interactions with nuclei consists only of the bound neutrons. |
| `per_proton`  | As for `per_neutron` but for processes that can only occur with target protons. |
| **Density Scaling** | |
| `per_bin_width` | Include no explicit additional scaling on the total rate for calculated neutrino--target interactions. This will often correspond to a cross section per elemental nucleus, such as carbon. It also covers simple elemental ratios commonly used for hydrocarbon targets, such as CH, CH2, or CH4. It can also be used for more complex molecular targets. |

The assumed, or default, value for this qualifier, following the majority of published data, is `cross_section_units=1E-38cm2|per_target|per_bin_width`

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

## 6. Smearing

Publishing non-true space measurements and smearing procedures is relatively rare, but is becoming more common thanks to the desireable statistical properties of techniques like Wiener SVD unfolding. The only valid _Qualifier_ for a smearing table is `smearing_type=smearing_matrix`. The _Independent Variable_ should be defined as in [Errors](#errors).

## Analysis Snippets

Analysis Snippets are relatively short C++ source files that contain implementations of event selection and projection operators. These should be written to work with the [ProSelecta](https://github.com/NUISANCEMC/ProSelecta) environment. Generally they are included as a single _Additional Resource_ file per record, which contains all of the functions used by that record.

### Function Naming Conventions

To avoid problems that would be encountered from sequentially loading multiple analysis snippets containing identically named functions, we provide a naming convention for selection and projection functions that should be followed where possible.

| Function Type | Example | Naming Convention | 
| ------------- | --------- | ----------------- |
| Selection | `int MicroBooNE_CC0Pi_2019_Selection_123456(HepMC3 const &)` | `<Experiment>_<MeasurementSpecifier>_<Year>_Select_<INSPIREHEPId>` |
| Projection |  `double MicroBooNE_CC0Pi_2019_DeltaPT_123456(HepMC3 const &)` | `<Experiment>_<MeasurementSpecifier>_<Year>_<ProjectionName>_<INSPIREHEPId>` |

# Tools and Utilities

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

# What To Do If My Measurement Doesn't Fit?

Weep profusely. To Write... 

In he meantime, reach out to [nuisance-owner\@projects.hepforge.org](mailto:nuisance-owner@projects.hepforge.org) or ask on [nuisance-xsec.slack.com](nuisance-xsec.slack.com).
