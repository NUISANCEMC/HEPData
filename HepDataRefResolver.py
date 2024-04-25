#!/usr/bin/env python3

import requests
from zipfile import ZipFile
import os
import sys

def ResolveRequestIdentifiers(hepdata_reference):
  reftype = "hepdata"
  recordid = None
  resourcename = None
  qualifier = None

  if '/' in hepdata_reference:
    record = hepdata_reference.split("/")[0]

    if ":" in record:
      reftype = record.split(":")[0]
      recordid = record.split(":")[1]
    else:
      recordid = record

    resource = hepdata_reference.split("/")[1]

    if ":" in resource:
      resourcename = resource.split(":")[0]
      qualifier = resource.split(":")[1]
    else:
      resourcename = resource

  elif ':' in hepdata_reference:
    reftype = hepdata_reference.split(":")[0]
    recordid = hepdata_reference.split(":")[1]
  else:
    recordid = hepdata_reference

  return { "reftype": reftype,
           "recordid": recordid,
           "resourcename": resourcename,
           "qualifier": qualifier }

def _build_local_HepData_Path(root_dir, recordid, record_version=1):
  return "/".join([root_dir, recordid, "HEPData-%s-v%s" % (recordid, record_version)])

def _resolve_resource(local_record_path, resourcename):
  if resourcename:
    if os.path.exists("%s/%s" % (local_record_path,resourcename)):
      return "%s/%s" % (local_record_path,resourcename)
    elif os.path.exists("%s/%s.yaml" % (local_record_path,resourcename)):
      return "%s/%s.yaml" % (local_record_path,resourcename)
    else:
      return None
  else:
    return local_record_path

def _tryotherhepdata(reftype, recordid):
  if reftype == "hepdata-sandbox":
    sub_check_response = requests.get("https://www.hepdata.net/record/%s" % (recordid), params={"format": "json"})
    if (sub_check_response.status_code == requests.codes.ok) and (sub_check_response.headers["content-type"] == "application/json"):
      return "Sandbox HepData recordid=%s doesn't seem to exist, but there is a normal record with that id. Try hepdata:%s" % \
          (recordid, recordid)

  elif reftype == "hepdata":
    sub_check_response = requests.get("https://www.hepdata.net/record/sandbox/%s" % (recordid), params={"format": "json"})

    if (sub_check_response.status_code == requests.codes.ok) and (sub_check_response.headers["content-type"] == "application/json"):
      return "Normal HepData recordid=%s doesn't seem to exist, but there is a sandbox record with that id. Try hepdata-sandbox:%s" % \
        (recordid, recordid)
  return None

def ResolveHepDataReference(root_dir, recordid, record_version=1, resourcename=None, qualifier=None, *args, **kwargs):
  local_record_versioned_path = _build_local_HepData_Path(root_dir, recordid, record_version)
  local_resource_path = _resolve_resource(local_record_versioned_path, resourcename)
  if local_resource_path and os.path.exists(local_resource_path):
    return local_resource_path

  # we don't have it and need to fetch it

  if kwargs["reftype"] == "hepdata-sandbox":
    record_URL = "https://www.hepdata.net/record/sandbox/%s" % (recordid)
  elif kwargs["reftype"] == "hepdata":
    record_URL = "https://www.hepdata.net/record/%s" % (recordid)

  sub_response = requests.get(record_URL, params={"format": "json"})

  if sub_response.status_code != requests.codes.ok:
    testother = _tryotherhepdata(kwargs["reftype"], recordid)
    if testother:
      raise RuntimeError(testother)

    raise RuntimeError("HTTP error response %s to GET: %s" % (sub_response.status_code, sub_response.url))

  if sub_response.headers["content-type"] != "application/json":
    testother = _tryotherhepdata(kwargs["reftype"], recordid)
    if testother:
      raise RuntimeError(testother)

    raise RuntimeError("Unexpected response to GET %s. Response content-type: %s. Does record %s exist?" % \
      (sub_response.url, sub_response.headers["content-type"], recordid) )

  submission = sub_response.json()
  
  table_names = [ x["name"] for x in submission["data_tables"] ]
  record_version = submission["version"]

  local_record_path = "%s/%s" % (root_dir, recordid)
  if not os.path.exists(local_record_path):
    print("record database path: %s does not exist" % local_record_path)
    os.makedirs(local_record_path)

  local_record_versioned_path = _build_local_HepData_Path(root_dir, recordid, record_version)

  zipsub_path = "%s.zip" % local_record_versioned_path
  if not os.path.exists(zipsub_path):
    submission_URL = "https://www.hepdata.net/download/submission/%s/%s/original" % (recordid, record_version)
    submission_zip_response = requests.get(submission_URL)
    
    if submission_zip_response.status_code != requests.codes.ok:
      raise RuntimeError("HTTP error response %s to GET: %s" % submission_zip_response.url)

    if submission_zip_response.headers["content-type"] != "application/zip":
      raise RuntimeError("Unexpected response to GET %s. Response content-type: %s, expected application/zip")

    with open(zipsub_path, 'wb') as fd:
      for chunk in submission_zip_response.iter_content(chunk_size=128):
        fd.write(chunk)

  if os.path.exists(zipsub_path):
    with ZipFile(zipsub_path, 'r') as zippedsub:
      zippedsub.extractall(local_record_versioned_path)
    os.remove(zipsub_path)
  else:
    raise RuntimeError("Expected %s to exist after downloading" % local_record_versioned_path)

  local_resource_path = _resolve_resource(local_record_versioned_path, resourcename)
  if local_resource_path:
    return local_resource_path

  raise RuntimeError("Expected resource %s to exist in %s" % (resourcename, local_record_versioned_path))

def GetLocalPathToResource(outdir_root, reference, context={}):
  reqids = ResolveRequestIdentifiers(reference)

  if reqids["recordid"] == None:
    if "recordid" in context:
      reqids["recordid"] = context["recordid"]
    else:
      raise RuntimeError("No record id supplied in reference: \"%s\", but context also had no record id." % reference)

  if (reqids["reftype"] == "hepdata-sandbox") or (reqids["reftype"] == "hepdata"):
    return ResolveHepDataReference("%s/%s" % (outdir_root, reqids["reftype"]), **reqids)
  else:
    raise RuntimeError("Unresolvable reference type %s" % reqids["reftype"])

if __name__ == "__main__":

  dbpath = os.environ.get("NUISANCE_HEPDATA_DATABASE")

  if not dbpath:
    raise RuntimeError("NUISANCE_HEPDATA_DATABASE environment variable is not defined")

  if not os.path.exists(dbpath):
    raise RuntimeError("NUISANCE_HEPDATA_DATABASE=%s points to a non-existant path" % dbpath)

  print(GetLocalPathToResource(dbpath, sys.argv[1]))

  