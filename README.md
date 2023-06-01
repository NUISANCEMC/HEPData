# NUISANCE HepData Conventions

This document corresponds to version 0.1 of the NUISNACE HepData Conventions.

This document aims to provide a set of conventions on top of the establish [HepData format specification]()

# Table of Content

+ [HepData Records](#hepdata-records)
  - [Tables](#Tables)
    * [Dependent and Independent Variables](#dependent-and-independent-variables)
    * [Units](#units)
    * [Qualifiers](#qualifiers)
  - [Additional Resources](#additional-resources)
  - [Record References](#record-references)
    * [Intra-record](#intra-record)
    * [Inter-record](#inter-record)
    * [INSPIRE](#inspire)
+ [Publications](#publications)
  - [Datasets](#datasets)
    * [Selections](#selections)
    * [Projections](#projections)
    * [Multi-dimensional Data](#multi-dimensional-data)
    * [Errors](#errors)
  - [Flux Predictions](#flux-predictions)
    * [Neutrino Energy Cuts](#neutrino-energy-cuts)

# HepData Records

## Tables

### Dependent and Independent Variables

### Units

### Qualifiers

## Additional Resources

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
