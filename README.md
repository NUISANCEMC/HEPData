# NUISANCE HepData Conventions

This document corresponds to version 0.1 of the NUISNACE HepData Conventions.

This document aims to provide a set of conventions on top of the establish [HepData format specification]() that allow NUISANCE to automatically construct predictions for given datasets from simulated

# Table of Contents

+ [HepData Records](#hepdata-records)
  - [Tables](#tables)
    * [Independent Variables](#independent-variables)
    * [Dependent Variables](#dependentvariables)
    * [Qualifiers](#qualifiers)
    * [Types](#types)
    * [Formats](#formats)
  - [Additional Resources](#additional-resources)
    * Projection and Selection Snippets](#projection-and-selection-snippets)
    * Data Release Conversion Scripts](#data-release-conversion-scripts)
  - [Record References](#record-references)
    * [Intra-record](#intra-record)
    * [Inter-record](#inter-record)
    * [INSPIRE](#inspire)
+ [Publications](#publications)
  - [Datasets](#datasets)
    * [Selections](#selections)
    * [Projections](#projections)
    * [Multi-dimensional Data](#multi-dimensional-data)
    * [Binning Functions](#binning-functions)
    * [Errors](#errors)
    * [Covariances](#covariances)
  - [Flux Predictions](#flux-predictions)
    * [Neutrino Energy Cuts](#neutrino-energy-cuts)

# HepData Records

The top level data structure for a HepData submission is called a Record. It can be referenced by a unique Id number. This document will not unneccessarily detail the HepData format as it is authoratatively documented elsewhere. Records are described by one or more `YAML` files and can contain additional files in a range of format as additional resources.

## Tables

A HepData Table broadly corresponds to a set of axes (Independent Variables) and measurements or predictions over those axes (Dependent Variables).

### Qualifiers

HepData Qualifiers are Key-Value pairs attached to a table as meta-data. These conventions describe a number of Qualifiers that may or must be present for a table to be compliant and automatically consumeable by NUISANCE.

### Independent Variables

Each independent variable must be accompanied by a [Qualifier](#qualifier) with the same name on the Table object corresponding to the function that should be used by NUISANCE to project an event onto that independent variable. This function must be defined in the [Snippet](#projection-and-selection-snippets) file, must have a unqiue but descriptive name.

#### Units

The units, where applicable, for each independent variable axis must be clearly defined in the appropriate HepData metadata. The units returned by the [Projection](#projections) functions described below must match the units declared on the independent variable axis, no units parsing or automatic conversion will be performed.

### Dependent Variables

#### Units

_LP: CrossSection Units. Do we want to be rigid about a format for the 'Units' bit, or add qualifiers for a few things that we want to handle explicitly? I would probably go with the latter. Treat the Units line as free-form text that we expect them to get right for prettyfication but have an explicit qualifier with options to describe any scale factors that we need to know about_

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

These conventions strongly recommend cross-sections be published PerTarget and in pico barns, _i.e._ `CrossSectionUnits=PerTarget|pb`

### Types

While many important features of a measurement can be guessed from the data contained within a table, it is useful to define a set of Types that can be used by NUISANCE as _hints_ as to how to construct a prediction of a measurement and make comparisons to the data. For a fully compliant NUISANCE HepData Table one `BinningType` Qualfier must exist and an optional `MeasurementType` Qualifer may optionally exist. If no `MeasurementType` is specified, then NUISANCE will default to treating the measurement as a `FluxAveragedDifferentialCrossSection` Type.

We want to implement the minimum set of specific cases that cover the majority of existing and envisioned measurement types, without the expectation that every single measurement will fit in one of our pre-defined types. For other types of measurements, many of the conventions in this document can still be followed to enable a succinct custom NUISANCE implementation through extending one of these types or compositing existing utilities.

The current list of implemented Types are detailed below, any table that declares and implements one of these Types should be automatically consumeable by NUISANCE:

#### BinningType

* `1D`
  - A one dimensional binned measurement with a single contiguous [Independent Variable](#independent-variables) axis. 
  - This corresponds to a ROOT `TH1`.
* `UniformND`
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
  - The most common type of published neutrino-scattering measurement. A prediction is made by selecting and projecting simulated neutrino interactions from one or more neutrino species drawn from a neutrin energy spectra, or flux shape.
* `EventRate`
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

## Flux Predictions

The vast majority of published neutrino-scattering data is flux-averaged or flux-integrated, which implies that a specific neutrino energy spectrum (or spectra for data with multiple neutrino species) should be used when generating predictions of the measurement from simulation. 

* Each [Table](#tables) must [Reference](#record-references) a specific flux prediction using [Qualifiers](#qualifiers) of the form `flux[-<BM>][-<NS>]=<TableRef>`, where: 
  * `<BM>` refers to the neutrino beam mode of the specified flux shape and should be one of `nu` or `anu` for neutrino- and antineutrino-enhanced beam modes respectively (sometimes referred to as FHC and RHC). This portion of the qualifier key may be omitted if only a single neutrino beam mode is required to generate a prediction of the measurement.
  * `<NS>` refers to the neutrino species in the explicit or implied beam mode and should be one of `numu`, `numubar`, `nue`, `nuebar`, `nutau`, `nutaubar`. This portion of the qualifier key may be omitted if only a single neutrino species is required to generate a prediction of the measurement.

### Neutrino Energy Cuts
