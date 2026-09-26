# Directory for building second version of SolTrace API, stapi_v2.

### Build

In `~/stapi_v2`, run `python -m build --wheel`. The `Microsoft.CppBuild.targets(548,5): warning MSB8029` seems to be harmless.
Once built, a path is set for searching for ptx files to a temporary directory. Rebuilding the SolTrace locally will set that to the typical build path again.

Note:
View wheel contents on Windows: `Add-Type -A "System.IO.Compression.FileSystem"; [IO.Compression.ZipFile]::OpenRead("C:\abs\path\to\SolTrace\coretrace\stapi_v2\dist\pysoltrace-0.1.0-cp314-cp314-win_amd64.whl").Entries.FullName`
Linux: unzip -l pysoltrace-0.1.0-cp314-cp314-win_amd64.whl

View dll contents on Windows in Developer PowerShell: `dumpbin /exports stapi_v2.dll`
