# NUISANCE HEPData Conventions

This document corresponds to version 0.1 of the [NUISANCE](https://github.com/NUISANCEMC/nuisance) HEPData Conventions.

This document aims to provide a set of conventions on top of the established [HEPData format specification](http://hepdata-submission.readthedocs.io) that allow NUISANCE to automatically construct predictions for datasets from simulated vectors of neutrino interactions. These conventions should not limit the type of measurements that can be expressed and are meant to cover the majority of existing published data. 

We want to implement the minimum set of specific cases that cover the majority of existing and envisioned measurement types, without the expectation that every single measurement will fit in one of our pre-defined types. For other types of measurements, many of the conventions in this document can still be followed to enable a succinct custom NUISANCE implementation through extending one of these types or compositing existing utilities. See [What To Do If My Measurement Doesn't Fit?](#what-to-do-if-my-measurement-doesn-t-fit).

# Table of Contents

+ [Checklist](#checklist)
+ [HEPData Records](#hEPdata-records)
  - [Tables](#tables)
    * [Qualifiers](#qualifiers)
    * [Types](#types)
    * [Independent Variables](#independent-variables)
    * [Dependent Variables](#dependent-variables)
    * [Formats](#formats)
  - [Additional Resources](#additional-resources)
    * [Projection and Selection Snippets](#projection-and-selection-snippets)
    * [Data Release Conversion Scripts](#data-release-conversion-scripts)
  - [Record References](#record-references)
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

* [✅] For each Independent Variable in a table a Qualifier must exist with the same name as the variable and a value corresponding to the name of a projection function in a snippet file included as an additional resource. See [Projection and Selection Snippets](#projection-and-selection-snippets).
* [✅] Each table must include a least one `Flux` qualifier. See [Flux Predictions](#flux-predictions).
* [✅] Each table must include a least one `Target` qualifier.
* [✅] Each table corresponding to a cross section measurement should include one `CrossSectionUnits` qualifier. See [Cross Section Units](#cross-section-units)
* [✅] Measurements that include a covariance estimate must include a `Covariance` qualifier. See [Errors](#errors).

# HEPData Records

The top level data structure for a HEPData submission is called a Record. It can be referenced by a unique Id number. This document will not unneccessarily detail the HEPData format as it is authoratatively documented elsewhere. Records are described by one or more `YAML` files and can contain additional files in a range of format as additional resources.

## Tables

A HEPData Table broadly corresponds to a set of axes (Independent Variables) and measurements or predictions over those axes (Dependent Variables).

### Qualifiers

HEPData Qualifiers are Key-Value pairs attached to a table as metadata. These conventions describe a number of Qualifiers that may or must be present for a table to be compliant and automatically consumeable by NUISANCE.

### Types

While many important features of a measurement can be guessed from the data and metadata contained within a table, it is useful to define a set of Types that can be used as _hints_ as to how to construct a prediction of a measurement and make comparisons to the data. For each HEPData Table one `MeasurementType` Qualifer may optionally exist. If no `MeasurementType` is specified, then the measurement type will default to `FluxAveragedDifferentialCrossSection` Type, which covers the vast majority of published data.

The current list of implemented Types are detailed below, any table that declares and implements one of these Types should be automatically consumeable by NUISANCE:

* `FluxAveragedDifferentialCrossSection`
  - The most common type of published neutrino-scattering measurement. A prediction is made by selecting and projecting simulated neutrino interactions from one or more neutrino species drawn from a neutrino energy spectra, or flux shape.
* `EventRate`
  - Some historic data are presented as simple event rates. For this case, only the shape of simulated predictions can be compared
* `SingleRatio`
  - Ratios must be treated specially as two predictions must be fully made before taking the ratio, they cannot be built up in a purely simulated-event-by-simulated-event way.
* `TotalCrossSection`
  - Some historic measurements attempt to unfold the neutrino flux shape and neutrino energy reconstruction effects from observations to make direct measurement of the total scattering cross section as a function of neutrino energy. While it is now clear that this approach is fraguht with simulation-dependence error mis-estimation problems, there are a number of data sets that are important to preserve. 
  - Predictions are made by selecting and projecting simualted neutrino interactions as a function of true neutrino energy, before accounting for the simulated flux integral in each energy bin. Since these require explicit cuts on the true neutrino energy additional snippet files also need to be provided.

This list may be extended in future versions.

When a measurement has multiple tables qualifiers should be provided for ALL tables such that a single HEPDATA table provides a self-contained summary of all analysis criteria associated with that data.

### Independent Variables

Each independent variable must be accompanied by a [Qualifier](#qualifier) with the same name on the Table object corresponding to the function that should be used to project an event onto that independent variable. This function must be defined in the [Snippet](#projection-and-selection-snippets) file and must have a unqiue but descriptive name.

#### Binning

The HEPData table format allows for fully generic hyper-rectangular bins in any number of dimensions. This is generic enough for any measurement that the authors are aware of. If your measurement makes use of non-hyper-rectangular bins, see [What To Do If My Measurement Doesn't Fit?](#what-to-do-if-my-measurement-doesn-t-fit) for ideas.

#### Units

The units, where applicable, for each independent variable axis must be clearly defined in the appropriate HEPData metadata. The units returned by the [Projection](#projections) functions described below must match the units declared on the independent variable axis, no units parsing or automatic conversion will be performed.

### Dependent Variables

Measurements may have one or more dependent variable which describe the measured cross section, predicted background contributions, predicted selection efficiencies, or a number of other constructions defined over the axes described by the list of [Independent Variables](#independent-variables). These conventions strongly encourage the dissemination of as much useful information included in making a measurement as possible, for fully automatic quantitative comparisons, the first dependent variable will be considered the measured data by default.

_PS: Agree with the latter too, but should avoid many qualifiers needing to be added. I really like your suggestion below of PerTarget|pb|PerBinWidth. I am unsure whether the bin scalings should be here or elsewhere?_
_PS: Are we assuming each PerTarget as just a factor? Do we need to include this as a qualifier too? E.g. NNucleons=16_
_LP: I think that we need a target specifier that we can parse to NNucleons. However, I think that we will also specify this on the 'other' end at input reading time and make NUISANCE's internal scaling PerTarget. Its just a lot more sensible all around (to me)._

#### Cross Section Units

For the majority of published data, as described in [Types](#types), the measurement will take the form of a scattering cross section. There are a number of historic conventions for the units and additional target material scale factors used in published cross section measurements. To avoid clumsy generic units metadata parsing, the explicit units should be also declared in a fixed form in a `CrossSectionUnits` [Qualifier](#qualifier).

* Per NTargets type, at most one of:
  - `PerTarget`
    + Include no explicit additional scaling on the total rate for calculated neutrino--target material interactions. This will often correspond to a cross section per elemental nucleus, such as carbon. It also covers simple elemental ratios commonly used for hydrocarbon targets, such as CH, CH2, or CH4. I can also be used for more complex molecular targets.
  - `PerNucleon`
    + Existing neutrino-scattering simulations often report the cross-section per target nucleon and the neutrino--nucleon interaction is considered the dominant fundamental process.
  - `PerNeutron`
    + Some data reports the cross-section per 'active' nucleon, which for muon neutrino CCQE interactions with nuclei consists only of the bound neutrons.
  - `PerProton`
    + As for PerNeutron but for processes that 

* Cross section units, at most one of:
  - `pb`
    + 1 pb = 1E-36 cm2
  - `nb`
    + 1 pb = 1E-33 cm2
  - `cm2`
    + A common unit for cross-section measurements that requires us to carry around a power-of-ten scale factor that is dangerously close to the [minimum value](https://en.wikipedia.org/wiki/Single-precision_floating-point_format) representable by single precision IEEE 754 floating point numbers.
  - `1E-38cm2`
    + Tries to avoid the 1E-38 factor by including it in the unit definition.

_LP: Do we want this to be explicit? It collides somewhat with the Measurement Type. A proper diffxsec will always divide by bin hyper-volume and an event rate should be shape-only._

* Bin scalings, at most one of:
  - `PerBinWidth` 
    + Explicitly divide the bin entries by bin width when calculating the cross-section.
  - `ShapeOnly`
    + Explicitly scale bin entries by its integral to produce a shape-only prediction when producing an MC histogram.

These conventions very strongly recommend cross sections be published PerTarget and in pico barns, _i.e._ `CrossSectionUnits=PerTarget|pb|PerBinWidth`. We strongly recommend explicitly including this qualifier, if one does not exist, the default recommendation will be assumed.

### Formats

Tables should generally be uploaded in the [HEPData `YAML` data format](https://hEPdata-submission.readthedocs.io/en/latest/data_yaml.html#yaml-data-file-example) as it allows unambiguous, fully generic N-dimensional rectangular binned and unbinned data. Using ROOT files containing `TH1` instances, as is common in HEP data releases, places additional constraints on the uniformity of multi-dimensional binnings that do not accomodate current and future measurements where more-optimized binning might be needed. However, given the ubiquity of ROOT histograms in existing data releases, they can be used, and HEPData can often transparently convert them to YAML files.

This document may be updated in the future to cover other on-disk formats that NUISANCE can handle, please get in touch if you would like to contribute or request a new data table format.

## Additional Resources

### Projection and Selection Snippets

### Data Release Conversion Scripts

## Record References

We define a format for record references as below, where `[]` denote optional components, `<>` denote the reference data itself, and `=` specifies a default value:

```
[<type=hEPdata>:][<id>][/<resource[:<qualifier>]>]
```

All parts of the reference are optional. In the absence of a reference `id`, the reference is considered an *intra*-record reference, referring to a resource contained within the same HEPData record. In the absence of a `type`, the reference is considered an *inter*-record reference, referring to a resource contained within another HEPData record. In the absence of a `resource` component, the reference is considered a generic link to another record and not a pointer to a specific resource of that record.

The `type` of a reference is freeform, but outside of the special (and default) `hEPdata` type a generic referred resource will not be automatically retrievable. Common, useful types might include: `INSPIRE`, `zenodo`, `doi`, among others. To refer to a specific resource, such as a flux prediction or covariance matrix, the `resource` component should be used. The `qualifier` sub-component is resource specific and is included to enable referring to sub-resources. 

Some specific examples with explanations are given below:

* `12345/MyCrossSection`: This reference refers to a table named `MyCrossSection` in HEPData record `12345`.
* `12345/MyCrossSection:BackgroundPrediction`: This reference refers specifically to the `BackgroundPrediction` [Dependent Variable](#dependent-variable) in table `MyCrossSection` in HEPData record `12345`.
* `12345/additionaldata.yaml`: This reference refers to an [Additional Resource File](#additional-resources) named `additionaldata.yaml` in HEPData record `12345`.
* `12345/flux.root:flux_numu`: This reference refers to a specific object (in this case a histogram) named `flux_numu` in [Additional Resource File](#additional-resources) `flux.root` in HEPData record `12345`.

#### A Note on HEPData sandboxed records

Because the HEPData REST API differentiates between public and sandboxed records, a separate type, `hEPdata-sandbox`, must be defined to enable access to records that are in the sandbox. Public records should never link to sandboxed records, but sandboxed records may link to either other sandboxed records or public records.

### Reference utilities

See [HepDataRefResolver.py](HepDataRefResolver.py) for utilities for resolving, fetching, and checking references.

# Publications

## Datasets

### Selections

### Projections

### Multi-dimensional Data

### Errors

_PS: For large matrices that are not invertible, do we want to suggest they check it can be inverted? Or provide an inverted one?_
_LP: I think we can leave the option for a pre-inverted one._

The [HEPData `YAML`]() supports the inclusion of multiple uncertainties for each bin as well as the linking to associated covariance matrices for each table. In all cases the qualifiers for a table should provide a reference to the recommended covariance matrix to use when calculating a test statistic with the data. If multiple covariances are provided that need to be added together then all covariance can be included in the submission as seperate tables, however a total covariance should also be uploaded alongside this. For the avoidance of doubt, a qualifier reference should be provided even if only one covariance is included in the submission : `Covariance=covariance_analysis1_total.dat`.

Errors should be included wherever possible in [HEPData `YAML`]() data tables even if there is an associated covariance. For event rate measurements where the known error is derived from Poisson statistics a calculated uncertainty should still be included.

### Test Statistics

_PS: Might need a qualifier for recommended test statistics, particularly old MiniBoone where its shape-only + Norm, ANL where its poission errors, etc_

If no test statistic qualifier is provided then the default statistic for any measurement when comparing it to a generator prediction is assumed to be a chi2 calculation of the form 
$$\chi2 = (d_i - m_i) * M_{ij}^{-1} * (d_j - m_j),$$
 where $d_i$ is the data in the ith bin, $m_i$ is the prediction in the ith bin, and $M_{ij}$ is the invert of the associated dataset covariance matrix. 

In some exceptional circumstances alternative test statistics may need to be suggested. If so these should be explicitly defined in the qualifiers for each table.

Include a qualifier like: `TestStatistic=Chi2`

* `ShapeOnlyChi2`
* `ShapeOnlyPlusNormChi2`
* `PoissionTestStatistic`

## Flux Predictions

The vast majority of published neutrino-scattering data is flux-averaged or flux-integrated, which implies that a specific neutrino energy spectrum (or spectra for data with multiple neutrino species) should be used when generating predictions of the measurement from simulation. 

* Each [Table](#tables) must [Reference](#record-references) a specific flux prediction using [Qualifiers](#qualifiers) of the form `Flux[-<BM>][-<NS>]=<TableRef>`, where: 
  * `<BM>` refers to the neutrino beam mode of the specified flux shape and should be one of `nu` or `anu` for neutrino- and antineutrino-enhanced beam modes respectively (sometimes referred to as FHC and RHC). This portion of the qualifier key may be omitted if only a single neutrino beam mode is required to generate a prediction of the measurement.
  * `<NS>` refers to the neutrino species in the explicit or implied beam mode and should be one of `numu`, `numubar`, `nue`, `nuebar`, `nutau`, `nutaubar`. This portion of the qualifier key may be omitted if only a single neutrino species is required to generate a prediction of the measurement.

* If multiple flux tables are provided for a single table then it should be assumed that events should be generated for all tables and combined based on their relative magntiude. 
* Where possible flux tables should be specified as the exact flux prediction used for the given measurement when the analysis took place.  _PS: On the fence about this..._
_LP: I think we need to make this recommendation, T2K analyses that 'fit' the flux out for a given xsec measurement, we've seen that by not giving the post fit flux CV you can screw up comparisons by using the general flux prediction blindly. This might be a detail that we could include if we turn this into a tech paper._

* Flux prediction tables should include a `bin_content_type` qualifier, that must have one of two values:

  * `count`: The histogram is a normal histogram where the bin height corresponds to the sum of weights within the bin
  * `count_per_bin_width`: The bin height for each bin corresponds to the sum of weights in that bin divided by the width of the bin (or some reference width, as when the height units look like: `events /50 MeV`).

### Neutrino Energy Cuts

# What To Do If My Measurement Doesn't Fit?

Weep profusely
