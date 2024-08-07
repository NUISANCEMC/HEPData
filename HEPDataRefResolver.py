#!/usr/bin/env python3

from zipfile import ZipFile
import os
import sys
import copy
import logging

import requests

logger = logging.getLogger("HepDataRefResolver")

def ResolveReferenceIdentifiers(hepdata_reference, **context):

  logger.info(f"ResolveReferenceIdentifiers({hepdata_reference},{context})")

  refcomps = {
    "reftype": str(context.get("reftype","")),
    "recordid": str(context.get("recordid","")),
    "recordvers": str(context.get("recordvers","")),
    "resourcename": str(context.get("resourcename","")),
    "qualifier": str(context.get("qualifier",""))
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
    raise RuntimeError(f"Didn't resolve a type for reference \"{hepdata_reference}\", with context: {context}")

  if not refcomps["recordid"]:
    raise RuntimeError(f"Didn't resolve a recordid for reference \"{hepdata_reference}\", with context: {context}")

  logger.info(f"ResolveReferenceIdentifiers({hepdata_reference},{context}) -> {refcomps}")

  return refcomps

def _build_local_HepData_path(root_dir, recordid, recordvers, **kwargs):
  return "/".join([root_dir, recordid, f"HEPData-{recordid}-v{recordvers}"])

def _resolve_resource_yaml_path(local_record_path, resourcename, **kwargs):
  if resourcename:
    if os.path.exists(f"{local_record_path}/{resourcename}"):
      return f"{local_record_path}/{resourcename}"
    elif os.path.exists(f"{local_record_path}/{resourcename}.yaml"):
      return f"{local_record_path}/{resourcename}.yaml"
    else:
      return None
  else:
    return local_record_path

def _tryotherhepdata(reftype, recordid, **kwargs):
  if reftype == "hepdata-sandbox":
    sub_check_response = requests.get(f"https://www.hepdata.net/record/{recordid}", params={"format": "json"})
    if (sub_check_response.status_code == requests.codes.ok) and (sub_check_response.headers["content-type"] == "application/json"):
      return f"Sandbox HepData recordid={recordid} doesn't seem to exist, but there is a normal record with that id. Try hepdata:{recordid}"

  elif reftype == "hepdata":
    sub_check_response = requests.get(f"https://www.hepdata.net/record/sandbox/{recordid}", params={"format": "json"})

    if (sub_check_response.status_code == requests.codes.ok) and (sub_check_response.headers["content-type"] == "application/json"):
      return f"Normal HepData recordid={recordid} doesn't seem to exist, but there is a sandbox record with that id. Try hepdata-sandbox:{recordid}"
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
    record_URL = f"https://www.hepdata.net/record/sandbox/{recordid}"
  elif reftype == "hepdata":
    record_URL = f"https://www.hepdata.net/record/{recordid}"

  sub_response = requests.get(record_URL, params={"format": "json"})

  if sub_response.status_code != requests.codes.ok:
    testother = _tryotherhepdata(reftype, recordid)
    if testother:
      raise RuntimeError(testother)

    raise RuntimeError(f"HTTP error response {sub_response.status_code} to GET: {sub_response.url}")

  if sub_response.headers["content-type"] != "application/json":
    testother = _tryotherhepdata(reftype, recordid)
    if testother:
      raise RuntimeError(testother)

    raise RuntimeError(f"Unexpected response to GET {sub_response.url}. Response content-type: {sub_response.headers['content-type']}. Does record {recordid} exist?")

  submission = sub_response.json()

  if not "version" in submission:
    raise RuntimeError(f"Response json does not look like a submission object: {submission}")

  if not recordvers:
    recordvers = submission["version"]
  elif int(recordvers) > int(submission["version"]):
    raise RuntimeError(f"Requested version {recordvers} of record {recordid}, but the record reported that the latest version is {submission['version']}.")

  local_record_root = f"{root_dir}/{recordid}"
  if not os.path.exists(local_record_root):
    logger.info(f"Making local record directory: {local_record_root}")
    os.makedirs(local_record_root)

  local_record_path = _build_local_HepData_path(root_dir, recordid, recordvers)

  zipsub_path = f"{local_record_path}.zip" 
  if not os.path.exists(zipsub_path):
    submission_URL = f"https://www.hepdata.net/download/submission/{recordid}/{recordvers}/original"
    submission_zip_response = requests.get(submission_URL)
    
    if submission_zip_response.status_code != requests.codes.ok:
      raise RuntimeError(f"HTTP error response code {submission_zip_response.status_code} to GET: {submission_zip_response.url}")

    if submission_zip_response.headers["content-type"] != "application/zip":
      raise RuntimeError(f"Unexpected response to GET. Response content-type: {submission_zip_response.headers['content-type']}, expected application/zip")

    with open(zipsub_path, 'wb') as fd:
      for chunk in submission_zip_response.iter_content(chunk_size=128):
        fd.write(chunk)

  if os.path.exists(zipsub_path):
    with ZipFile(zipsub_path, 'r') as zippedsub:
      zippedsub.extractall(local_record_path)
    os.remove(zipsub_path)
  else:
    raise RuntimeError(f"Expected {local_record_path} to exist after downloading and extracting")

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

  raise RuntimeError(f"Expected resource {resourcename} to exist in {local_record_path}")

def _build_local_INSPIREHEP_path(root_dir, recordid, recordvers, **kwargs):
  return "/".join([root_dir, "INSPIREHEP", recordid, f"submission-{recordid}"])

def ResolveINSPIREHEPReference(root_dir, **refcomps):

  reftype = refcomps.get("reftype")
  recordid = refcomps.get("recordid")
  recordvers = refcomps.get("recordvers")
  resourcename = refcomps.get("resourcename")
  qualifier = refcomps.get("qualifier")

  local_record_path = _build_local_INSPIREHEP_path(root_dir, **refcomps)
  local_resource_path = _resolve_resource_yaml_path(local_record_path, **refcomps)
  if local_resource_path and os.path.exists(local_resource_path):
    return local_record_path, local_resource_path, refcomps

  raise RuntimeError(f"Expected resource {resourcename} to exist in {local_record_path}")

def GetLocalPathToResource(outdir_root, reference, *args, **context):
  refcomps = ResolveReferenceIdentifiers(reference, **context)

  if (refcomps["reftype"] == "hepdata-sandbox") or (refcomps["reftype"] == "hepdata"):
    return ResolveHepDataReference("/".join([outdir_root, refcomps["reftype"]]), **refcomps)
  elif refcomps["reftype"] == "inspirehep":
    return ResolveINSPIREHEPReference(outdir_root, **refcomps)
  else:
    raise RuntimeError(f"Unresolvable reference type {refcomps['reftype']}")

if __name__ == "__main__":

  # logging.basicConfig(level=logging.INFO)

  dbpath = os.environ.get("HEPDATA_RECORD_PATH")

  try:
    if not dbpath:
      raise RuntimeError("HEPDATA_RECORD_DATABASE environment variable is not defined")

    if not os.path.exists(dbpath):
      raise RuntimeError("HEPDATA_RECORD_DATABASE=%s points to a non-existant path" % dbpath)

    print(GetLocalPathToResource(dbpath, sys.argv[1])[1])
  except RuntimeError as err:
    print(err)

  