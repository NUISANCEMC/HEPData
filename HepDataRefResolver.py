#!/usr/bin/env python3

from zipfile import ZipFile
import os
import sys
import copy
import logging

import requests

logger = logging.getLogger("HepDataRefResolver")

def ResolveReferenceIdentifiers(hepdata_reference, **context):

  logger.info("ResolveReferenceIdentifiers(%s,%s)" % (hepdata_reference, context))

  refcomps = {
    "reftype": context.get("reftype"),
    "recordid": str(context.get("recordid")),
    "recordvers": context.get("recordvers"),
    "resourcename": context.get("resourcename"),
    "qualifier": context.get("qualifier")
  }

  if hepdata_reference and isinstance(hepdata_reference, str): # empty references can still be meaningful in context
    if '/' in hepdata_reference:
      record = hepdata_reference.split("/")[0]

      if ":" in record:
        refcomps["reftype"] = record.split(":")[0]
        refcomps["recordid"] = record.split(":")[1]
      else:
        refcomps["recordid"] = record

      resource = hepdata_reference.split("/")[1]

      if ":" in resource:
        refcomps["resourcename"] = resource.split(":")[0]
        refcomps["qualifier"] = resource.split(":")[1]
      else:
        refcomps["resourcename"] = resource

    elif ':' in hepdata_reference:
      refcomps["reftype"] = hepdata_reference.split(":")[0]
      refcomps["recordid"] = hepdata_reference.split(":")[1]
    else:
      try:
        recordid = int(hepdata_reference)
        refcomps["recordid"] = str(recordid)
      except:
        refcomps["resourcename"] = hepdata_reference
  elif isinstance(hepdata_reference, int):
    refcomps["recordid"] = str(hepdata_reference)

  if 'v' in refcomps["recordid"]:
    recordid = refcomps["recordid"].split("v")[0]
    recordvers = refcomps["recordid"].split("v")[1]

    refcomps["recordid"] = recordid
    refcomps["recordvers"] = recordvers

  if not refcomps["reftype"]:
    raise RuntimeError("Didn't resolve a type for reference \"%s\", with context: %s" % (hepdata_reference, context))

  if not refcomps["recordid"]:
    raise RuntimeError("Didn't resolve a recordid for reference \"%s\", with context: %s" % (hepdata_reference, context))

  logger.info("ResolveReferenceIdentifiers(%s,%s) -> %s" % (hepdata_reference, context, refcomps))

  return refcomps

def _build_local_HepData_path(root_dir, recordid, recordvers, **kwargs):
  return "/".join([root_dir, recordid, "HEPData-%s-v%s" % (recordid, recordvers)])

def _resolve_resource_yaml_path(local_record_path, resourcename, **kwargs):
  if resourcename:
    if os.path.exists("%s/%s" % (local_record_path,resourcename)):
      return "%s/%s" % (local_record_path,resourcename)
    elif os.path.exists("%s/%s.yaml" % (local_record_path,resourcename)):
      return "%s/%s.yaml" % (local_record_path,resourcename)
    else:
      return None
  else:
    return local_record_path

def _tryotherhepdata(reftype, recordid, **kwargs):
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

def _tryversion1(reftype, recordid):
  return None

def ResolveHepDataReference(root_dir, **refcomps):

  reftype = refcomps.get("reftype")
  recordid = refcomps.get("recordid")
  recordvers = refcomps.get("recordvers")
  resourcename = refcomps.get("resourcename")
  qualifier = refcomps.get("qualifier")

  local_record_path = _build_local_HepData_path(root_dir, **refcomps)
  local_resource_path = _resolve_resource_yaml_path(local_record_path, **refcomps)
  if local_resource_path and os.path.exists(local_resource_path):
    return local_record_path, local_resource_path, refcomps

  # we don't have it and need to fetch it

  if reftype == "hepdata-sandbox":
    record_URL = "https://www.hepdata.net/record/sandbox/%s" % (recordid)
  elif reftype == "hepdata":
    record_URL = "https://www.hepdata.net/record/%s" % (recordid)

  sub_response = requests.get(record_URL, params={"format": "json"})

  if sub_response.status_code != requests.codes.ok:
    testother = _tryotherhepdata(reftype, recordid)
    if testother:
      raise RuntimeError(testother)

    raise RuntimeError("HTTP error response %s to GET: %s" % (sub_response.status_code, sub_response.url))

  if sub_response.headers["content-type"] != "application/json":
    testother = _tryotherhepdata(reftype, recordid)
    if testother:
      raise RuntimeError(testother)

    raise RuntimeError("Unexpected response to GET %s. Response content-type: %s. Does record %s exist?" % \
      (sub_response.url, sub_response.headers["content-type"], recordid) )

  submission = sub_response.json()

  if not "version" in submission:
    raise RuntimeError("Response json does not look like a submission object: %s" % submission)

  if not recordvers:
    recordvers = submission["version"]
  elif int(recordvers) > int(submission["version"]):
    raise RuntimeError("Requested version %s of record %s, but the record reported that the latest version is %s." % (recordvers, recordid, submission["version"]))

  local_record_root = "%s/%s" % (root_dir, recordid)
  if not os.path.exists(local_record_root):
    logger.info("Making local record directory: %s" % local_record_root)
    os.makedirs(local_record_root)

  local_record_path = _build_local_HepData_path(root_dir, recordid, recordvers)

  zipsub_path = "%s.zip" % local_record_path
  if not os.path.exists(zipsub_path):
    submission_URL = "https://www.hepdata.net/download/submission/%s/%s/original" % (recordid, recordvers)
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
      zippedsub.extractall(local_record_path)
    os.remove(zipsub_path)
  else:
    raise RuntimeError("Expected %s to exist after downloading and extracting" % local_record_path)

  local_resource_path = _resolve_resource_yaml_path(local_record_path, resourcename)

  if local_resource_path:
    updated_context = {
      "reftype": reftype,
      "recordid": recordid,
      "recordvers": recordvers,
      "resourcename": resourcename,
      "qualifier": qualifier
    }
    return local_record_path, local_resource_path, updated_context

  raise RuntimeError("Expected resource %s to exist in %s" % (resourcename, local_record_path))

def GetLocalPathToResource(outdir_root, reference, *args, **context):
  refcomps = ResolveReferenceIdentifiers(reference, **context)

  reftype_root_dir = "/".join([outdir_root, refcomps["reftype"]])

  if (refcomps["reftype"] == "hepdata-sandbox") or (refcomps["reftype"] == "hepdata"):
    return ResolveHepDataReference(reftype_root_dir, **refcomps)
  else:
    raise RuntimeError("Unresolvable reference type %s" % refcomps["reftype"])

if __name__ == "__main__":

  # logging.basicConfig(level=logging.INFO)

  dbpath = os.environ.get("HEPDATA_RECORD_DATABASE")

  try:
    if not dbpath:
      raise RuntimeError("HEPDATA_RECORD_DATABASE environment variable is not defined")

    if not os.path.exists(dbpath):
      raise RuntimeError("HEPDATA_RECORD_DATABASE=%s points to a non-existant path" % dbpath)

    print(GetLocalPathToResource(dbpath, sys.argv[1])[1])
  except RuntimeError as err:
    print(err)

  