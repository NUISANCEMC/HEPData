# NUISANCE HepData Conventions

This document corresponds to version 0.1 of the NUISNACE HepData Conventions.

This document aims to provide a set of conventions on top of the establish [HepData format specification]() that allow NUISANCE to automatically construct predictions for given datasets from simulated

# Table of Contents

- [NUISANCE HepData Conventions](#nuisance-hepdata-conventions)
- [Table of Contents](#table-of-contents)
- [HepData Records](#hepdata-records)
  - [Tables](#tables)
    - [Qualifiers](#qualifiers)
    - [Independent Variables](#independent-variables)
      - [Units](#units)
    - [Dependent Variables](#dependent-variables)
      - [Units](#units-1)
    - [Types](#types)
      - [BinningType](#binningtype)
      - [MeasurementType](#measurementtype)
    - [Formats](#formats)
  - [Additional Resources](#additional-resources)
    - [Projection and Selection Snippets](#projection-and-selection-snippets)
    - [Data Release Conversion Scripts](#data-release-conversion-scripts)
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
    - [Neutrino Energy Cuts](#neutrino-energy-cuts)

# HepData Records

The top level data structure for a HepData submission is called a Record. It can be referenced by a unique Id number. This document will not unneccessarily detail the HepData format as it is authoratatively documented elsewhere. Records are described by one or more `YAML` files and can contain additional files in a range of format as additional resources.

## Tables

A HepData Table broadly corresponds to a set of axes (Independent Variables) and measurements or predictions over those axes (Dependent Variables).

### Qualifiers

HepData Qualifiers are Key-Value pairs attached to a table as meta-data. These conventions describe a number of Qualifiers that may or must be present for a table to be compliant and automatically consumeable by NUISANCE.

When a measurement has multiple tables qualifiers should be provided for ALL tables such that a single HEPDATA table provides a self-contained summary of all analysis criteria associated with that data.

### Independent Variables

Each independent variable must be accompanied by a [Qualifier](#qualifier) with the same name on the Table object corresponding to the function that should be used by NUISANCE to project an event onto that independent variable. This function must be defined in the [Snippet](#projection-and-selection-snippets) file, must have a unqiue but descriptive name.

#### Units

The units, where applicable, for each independent variable axis must be clearly defined in the appropriate HepData metadata. The units returned by the [Projection](#projections) functions described below must match the units declared on the independent variable axis, no units parsing or automatic conversion will be performed.

### Dependent Variables

#### Units

_LP: CrossSection Units. Do we want to be rigid about a format for the 'Units' bit, or add qualifiers for a few things that we want to handle explicitly? I would probably go with the latter. Treat the Units line as free-form text that we expect them to get right for prettyfication but have an explicit qualifier with options to describe any scale factors that we need to know about_

_PS: Agree with the latter too, but should avoid many qualifiers needing to be added. I really like your suggestion below of PerTarget|pb|PerBinWidth. I am unsure whether the bin scalings should be here or elsewhere?_
_PS: Are we assuming each PerTarget as just a factor? Do we need to include this as a qualifier too? E.g. NNucleons=16_

Include a qualifier like: `CrossSectionUnits=<>[|<>]`

Options:

* Per NTargets type, one of:
  - `PerTarget`
    + Include no explicit additional scaling on the total rate for calculated neutrino--target material interactions.
  - `PerNucleus`
    + Include no explicit additional scaling on the total rate for calculated neutrino--nuclei interactions. Difficult to consistently define for non-elemental targets, such as commonly-used hydrocarbon active target materials.
  - `PerNucleon`
    + Existing neutrino-scattering simulations often report the cross-section per target nucleon and the neutrino--nucleon interaction is considered the dominant fundamental process.
  - `PerNeutron`
    + Some data reports the cross-section per 'active' nucleon, which for muon neutrino CCQE interactions with nuclei consists only of the bound neutrons.
  - `PerProton`
    + As for PerNeutron but for processes that 

* Cross section units, one of:
  - `pb`
    + 1 pb = 1E-36 cm2
  - `nb`
    + 1 pb = 1E-33 cm2
  - `cm2`
    + A common unit for cross-section measurements that requires us to carry around a power-of-ten scale factor that is dangerously close to the minimum value representable by single precision IEEE 754 floating point numbers.
  - `1E-38cm2`
    + Tries to avoid the 1E-38 factor by including it in the unit definition.

* Bin scalings, one of:
  - `PerBinWidth` 
    + Explicitly divide the bin entries by bin width when calculating the cross-section.
  - `ShapeOnly`
    + Explicitly scale bin entries by its integral to produce a shape-only prediction when producing an MC histogram.

These conventions strongly recommend cross-sections be published PerTarget and in pico barns, _i.e._ `CrossSectionUnits=PerTarget|pb`

### Types

While many important features of a measurement can be guessed from the data contained within a table, it is useful to define a set of Types that can be used by NUISANCE as _hints_ as to how to construct a prediction of a measurement and make comparisons to the data. For a fully compliant NUISANCE HepData Table one `BinningType` Qualfier must exist and an optional `MeasurementType` Qualifer may optionally exist. If no `MeasurementType` is specified, then NUISANCE will default to treating the measurement as a `FluxAveragedDifferentialCrossSection` Type.

We want to implement the minimum set of specific cases that cover the majority of existing and envisioned measurement types, without the expectation that every single measurement will fit in one of our pre-defined types. For other types of measurements, many of the conventions in this document can still be followed to enable a succinct custom NUISANCE implementation through extending one of these types or compositing existing utilities.

The current list of implemented Types are detailed below, any table that declares and implements one of these Types should be automatically consumeable by NUISANCE:

#### BinningType

* `1D` _PS: I think remove and put ContiguousND corresponds to THX_
  - A one dimensional binned measurement with a single contiguous [Independent Variable](#independent-variables) axis. 
  - This corresponds to a ROOT `TH1`.
* `UniformND` _PS: I think remove and put ContiguousND corresponds to THX_
  - A multi-dimensional binned measurement where the binning in any [Independent Variable](#independent-variables) does not change based on the value of another [Independent Variable](#independent-variables). 
  - The corresponds to a ROOT `THX` for X = 1, 2, 3, N.
* `ContiguousND`
  - A multi-dimensional binned measurement where the binning in each [Independent Variable](#independent-variables) can generically change based on the value of any other [Independent Variable](#independent-variables).
  - All bins in each [Independent Variable](#independent-variables) must be contiguous with two other bins except for two extremal 'end' bins which must be contiguous with only one bin each. This is equivalent to saying that there can be no internal gaps in the binning scheme.
  - _N.B._ This also implies that bins must not overlap.
* `NonContiguousND`
  - A generic ND measurement where bins are referenced by a single integer identifier, or global bin number, and a binning function must be provided to map any given set of [Independent Variable](#independent-variables) unambiguously to a single bin.
  - This is equivalent to binning by the _global_ bin number for a ROOT `THX`.

#### MeasurementType

* `FluxAveragedDifferentialCrossSection`
  - The most common type of published neutrino-scattering measurement. A prediction is made by selecting and projecting simulated neutrino interactions from one or more neutrino species drawn from a neutrino energy spectra, or flux shape.
* `EnergyExplicitCrossSection` _PS: Maybe this was TotalCrossSection_
  - Measurements of neutrino scattering cross-sections where the binning is explicitly given in terms of unfolded neutrino energy. Predictions are made by selecting and projecting simualted neutrino interactions as a function of true neutrino energy, before accounting for the simulated flux integral in each energy bin. Since these require explicit cuts on the true neutrino energy additional snippet files also need to be provided.
* `EventRate`
  - Raw event rate data in which no explicit normalisation data has been provided. Predictions are made by selecting and projection neutrino interactions before scaling the final event rate production to match that of the data to make a shape-only comparison.  Typically this only applies to older bubble chamber experiment data.
* `TotalCrossSection`
* `SingleRatio`

This list may be extended in future versions.

### Formats

Tables should generally be uploaded in the [HepData `YAML`]() format as it allows unambiguous, fully generic N-dimensional rectangular binned and unbinned data. Using ROOT files containing `TH1`instances, as is common in HEP data releases, places additional constraints on the uniformity of multi-dimensional binnings that do not accomodate current and future measurements where more-optimized binning might be needed. However, given the ubiquity of ROOT histograms in existing data releases, they can be used.

This document may be updated in the future to cover other on-disk formats that NUISANCE can handle, please get in touch if you would like to contribute or request a new data table format.

## Additional Resources

### Projection and Selection Snippets

### Data Release Conversion Scripts

## Record References

### Intra-record

### Inter-record

### INSPIRE

# Publications

## Datasets

### Selections

### Projections

### Multi-dimensional Data

### Errors

_PS: For large matrices that are not invertible, do we want to suggest they check it can be inverted? Or provide an inverted one?_

The [HepData `YAML`]() supports the inclusion of multiple uncertainties for each bin as well as the linking to associated covariance matrices for each table. In all cases the qualifiers for a table should provide a reference to the recommended covariance matrix to use when calculating a test statistic with the data. If multiple covariances are provided that need to be added together then all covariance can be included in the submission as seperate tables, however a total covariance should also be uploaded alongside this. For the avoidance of doubt a qualifier reference should be provided even if only one covariance is included in the submission : `Covariance=covariance_analysis1_total.dat`.

Errors should be included wherever possible in [HepData `YAML`]() data tables even if there is an associated covariance. For event rate measurements where the known error is derived from Poisson statistics a calculated uncertainty should still be included.

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

* Each [Table](#tables) must [Reference](#record-references) a specific flux prediction using [Qualifiers](#qualifiers) of the form `flux[-<BM>][-<NS>]=<TableRef>`, where: 
  * `<BM>` refers to the neutrino beam mode of the specified flux shape and should be one of `nu` or `anu` for neutrino- and antineutrino-enhanced beam modes respectively (sometimes referred to as FHC and RHC). This portion of the qualifier key may be omitted if only a single neutrino beam mode is required to generate a prediction of the measurement.
  * `<NS>` refers to the neutrino species in the explicit or implied beam mode and should be one of `numu`, `numubar`, `nue`, `nuebar`, `nutau`, `nutaubar`. This portion of the qualifier key may be omitted if only a single neutrino species is required to generate a prediction of the measurement.

* If multiple flux tables are provided for a single table then it should be assumed that events should be generated for all tables and combined based on their relative magntiude. 
* Where possible flux tables should be specified as the exact flux prediction used for the given measurement when the analysis took place.  _PS: On the fence about this...._

### Neutrino Energy Cuts
