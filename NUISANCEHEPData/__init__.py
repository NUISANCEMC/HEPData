import yaml

from .HEPDataRefResolver import *

measurement_variable_types = [ "cross_section_measurement", "composite_cross_section_measurement" ]

class NUISHEPDataFluxTable:
  def __init__(self, ref, ctx):
    self.record_database_root = os.environ.get("NUISANCEDB")
    self.ref = ref

    # ensure that a local copy of the resource exists
    _, self.resource_path, uctx = GetLocalPathToResource(self.record_database_root, ref, **ctx)

    with open(self.resource_path, 'r') as ytable:
      table_yaml = yaml.safe_load(ytable)

      depvar = None

      if uctx.get("qualifier"):
        for dv in table_yaml["dependent_variables"]:
          if depvar["header"]["name"] == uctx.get("qualifier"):
            depvar = dv
      else:
        depvar = table_yaml["dependent_variables"][0]

      kvpairs = { x["name"]: x["value"] for x in depvar["qualifiers"] }

      self.variable_type = kvpairs["variable_type"]

      if self.variable_type not in [ "probe_flux" ]:
        raise RuntimeError(str(f"Invalid variable_type qualifier value for NUISHEPDataFluxTable: {self.variable_type}"))

      self.probe_particle = kvpairs["probe_particle"]
      self.bin_content_type  = kvpairs["bin_content_type"]
      self.bins = [ (b["low"], b["high"]) for b in table_yaml["independent_variables"][0]["values"] ]
      self.values = list(depvar['values'])

  def tostr(self, indent=""):
    return f"""{{
{indent}  ref: {self.ref}
{indent}  nbins: {len(self.bins)}
{indent}  probe_particle: {self.probe_particle}
{indent}  bin_content_type: {self.bin_content_type}
{indent}}}"""

  def __repr__(self):
    return self.tostr()

class NUISHEPDataErrorTable:
  def __init__(self, ref, ctx):
    self.record_database_root = os.environ.get("NUISANCEDB")

    # ensure that a local copy of the resource exists
    _, self.resource_path, uctx = GetLocalPathToResource(self.record_database_root, ref, **ctx)

    with open(self.resource_path, 'r') as ytable:
      table_yaml = yaml.safe_load(ytable)

      depvar = None

      if uctx.get("qualifier"):
        for dv in table_yaml["dependent_variables"]:
          if depvar["header"]["name"] == uctx.get("qualifier"):
            depvar = dv
      else:
        depvar = table_yaml["dependent_variables"][0]

      kvpairs = { x["name"]: x["value"] for x in depvar["qualifiers"] }

      self.variable_type = kvpairs["variable_type"]
      
      if self.variable_type != "error_table":
        raise RuntimeError(str(f"Invalid variable_type qualifier value for NUISHEPDataErrorTable: {self.variable_type}"))

      self.error_type = kvpairs["error_type"]

      if self.error_type not in [ "covariance", "correlation" ]:
        raise RuntimeError(str(f"Invalid error_type qualifier value for NUISHEPDataErrorTable: {self.error_type}"))


class NUISHEPDataDependentVariable:
  def __init__(self, indepvars, depvar, ctx):

    kvpairs = { x["name"]: x["value"] for x in depvar["qualifiers"] }

    self.indepvar = indepvars.copy()

    self.name = depvar["header"]["name"]

    self.variable_type = kvpairs["variable_type"]

    self.measurement_type = kvpairs.get("measurement_type", "flux_avgeraged_differential_cross_section")

    if self.measurement_type not in [ "flux_avgeraged_differential_cross_section", "event_rate", "ratio", "total_cross_section" ]:
      raise RuntimeError(str(f"Invalid measurement_type qualifier value for NUISHEPDataDependentVariable: {self.measurement_type}"))

    if self.variable_type == "cross_section_measurement":
      self.selectfunc = kvpairs["selectfunc"]
      self.projectfuncs = { v['name']: kvpairs[f"{v['name']}:projectfunc"] for v in indepvars }
      self.cross_section_units = kvpairs.get("cross_section_units", "pb|per_target|per_bin_width").split("|")

      xs_base_units = [ "pb", "nb", "cm2", "10E-38cm2" ]
      xs_target_units = [ "per_target", "per_nucleon", "per_neutron", "per_proton" ]
      xs_bin_units = [ "per_bin_width", "per_first_bin_width" ]
      xs_allunits = xs_base_units[:] + xs_target_units + xs_bin_units

      for xs_flag in self.cross_section_units:
        if xs_flag not in xs_allunits:
          raise RuntimeError(str(f"Invalid cross_section_units qualifier flag for NUISHEPDataDependentVariable: {xs_flag}"))

      self.target = kvpairs["target"]
      self.probe_flux = kvpairs["probe_flux"]

      self.flux = NUISHEPDataFluxTable(self.probe_flux, ctx)
    else:
      self.record_database_root = os.environ.get("NUISANCEDB")

      sub_measurement_refs = kvpairs["sub_measurements"].split(",")
      self.sub_dependent_variables = []

      for smr in sub_measurement_refs:
        # ensure that a local copy of the record exists
        _, sub_resource_path, sub_ctx = GetLocalPathToResource(self.record_database_root, smr, **ctx)

        sub_measurement = None
        with open(sub_resource_path, 'r') as subdoc:
          tdoc = yaml.safe_load(subdoc)

          for depvar in tdoc["dependent_variables"]:
            if sub_ctx["qualifier"] and (depvar["header"]["name"] != sub_ctx["qualifier"]):
              continue

            for qual in depvar["qualifiers"]:
              if qual["name"] == "variable_type" and qual["value"] in measurement_variable_types:
                sub_measurement = NUISHEPDataMeasurementTable(sub_ctx["resourcename"], tdoc, sub_ctx)

        if not sub_measurement:
          raise RuntimeError(str(f"Failed to resolve sub_measurement reference:\"{smr}\" to a dependent variable on another table"))

        for dv in sub_measurement.dependent_variables:
          if sub_ctx["qualifier"]:
            if dv.name == sub_ctx["qualifier"]: 
              self.sub_dependent_variables.append(dv)
              break
          else:
            self.sub_dependent_variables.append(dv)
            break

    self.test_statistic = kvpairs.get("test_statistic", "chi2")
    self.error = kvpairs.get("error")
    self.error_table = "" #NUISHEPDataErrorTable(self.error, ctx) if self.error else None
    self.smearing = kvpairs.get("smearing")
    self.smearing_table = "" #NUISHEPDataErrorTable(self.smearing, ctx) if self.smearing else None

  def tostr(self, indent=""):
    if self.variable_type == "cross_section_measurement":
      outstr = f"""{{ 
{indent}  variable_type: {self.variable_type}
{indent}  selectfunc: {self.selectfunc}
{indent}  projectfuncs: {{
"""
      for var, ref in self.projectfuncs.items():
        outstr += indent + f"    {var}: \'{ref}\',\n"
      outstr += f"""{indent}  }}
{indent}  cross_section_units: {self.cross_section_units}
{indent}  target: {self.target}
{indent}  flux: {self.flux.tostr(indent + '  ')}
{indent}  test_statistic: {self.test_statistic}
{indent}  error_table: {self.error_table}
{indent}  smearing_table: {self.smearing_table}
{indent}}}"""
      return outstr
    else:
      outstr = f"""{{
{indent}  variable_type: {self.variable_type}
{indent}  sub_dependent_variables: [
"""
      for dv in self.sub_dependent_variables:
        outstr += indent + "    " + dv.tostr(indent + "    ") + ",\n"
      outstr += f"""{indent}  ]
{indent}  test_statistic: {self.test_statistic}
{indent}  error_table: {self.error_table}
{indent}  smearing_table: {self.smearing_table}
{indent}}}"""
      return outstr

  def __repr__(self):
    return self.tostr()

class NUISHEPDataMeasurementTable:
  def __init__(self, name, table, ctx):

    self.name = name

    self.independent_variables = []
    for indepvar in table["independent_variables"]:
      self.independent_variables.append(
          { "name": indepvar["header"]["name"],
            "units": indepvar["header"].get("units"),
            "values": list(indepvar["values"])
          }
        )

    self.dependent_variables = []
    for depvar in table["dependent_variables"]:
      kvpairs = { x["name"]: x["value"] for x in depvar["qualifiers"] }
      if "variable_type" in kvpairs and kvpairs["variable_type"] in measurement_variable_types:
        self.dependent_variables.append(NUISHEPDataDependentVariable(self.independent_variables, depvar, ctx))

  def tostr(self, indent=""):
    outstr = f"""{{
{indent}  name: {self.name}
{indent}  independent_variables: [
"""
    for iv in self.independent_variables:
      outstr += f"{indent}    {iv['name']},\n"
    outstr += f"{indent}  ]\n"

    outstr += f"""{indent}  dependent_variables: [
"""
    for dv in self.dependent_variables:
      outstr += indent + "    " + dv.tostr(indent + "    ") + ",\n"
    outstr += f"{indent}  ]\n"
    outstr += f"{indent}}}"
    
    return outstr

  def __repr__(self):
    return self.tostr()

class NUISHEPDataRecord(object):
  def __init__(self, ref):
    self.record_database_root = os.environ.get("NUISANCEDB")

    # logging.basicConfig(level=logging.INFO)

    # ensure that a local copy of the record exists
    self.record_path, _, self.ctx = GetLocalPathToResource(self.record_database_root, ref)

    with open(f"{self.record_path}/submission.yaml", 'r') as yfile:
      submission_yaml = yaml.safe_load_all(yfile)

      self.cross_section_measurements = []
      self.additional_resources = []

      # loop through sub-documents in the submission file and get all named tables
      for doc in submission_yaml:
        if "data_file" in doc:
          docname = doc["name"]
          with open(f"{self.record_path}/{doc['data_file']}", 'r') as ydoc:
            table = yaml.safe_load_all(ydoc)

            for tdoc in table:
              for depvar in tdoc["dependent_variables"]:
                for qual in depvar["qualifiers"]:
                  if qual["name"] == "variable_type" and qual["value"] in measurement_variable_types:
                    self.cross_section_measurements.append(NUISHEPDataMeasurementTable(docname, tdoc, self.ctx))
        else:
          for addres in doc["additional_resources"]:
            self.additional_resources.append(addres["location"])

  def tostr(self, indent=""):
    outstr = f"""{indent}Record: {self.ctx['reftype']}:{self.ctx['recordid']}
{indent}  cross_section_measurements: [
"""
    for xsm in self.cross_section_measurements:
      outstr += indent + "    " + xsm.tostr(indent + "    ") + ",\n"
    outstr += f"{indent}  ]\n"

    outstr += f"""{indent}  additional_resources: [
"""
    for ar in self.additional_resources:
      outstr +=  f"{indent}    {ar},\n"
    outstr += f"{indent}  ]"

    return outstr

  def __repr__(self):
    return self.tostr()
