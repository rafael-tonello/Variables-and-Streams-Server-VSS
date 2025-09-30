# datetimes are not inside strings in the jsons returned by http api

When running the script /media/veracrypt/projects/rafinha_tonello/Home automation and related projects/datetime broadcast.sh, the jsons returned by the http api have the datetime values not inside strings, causing error in the JSON parsing by clients.

Example of set to replicate the error

1) run the command below to set a datetime in the var server

```bash
curl -X POST "http://vss.antares:5024/n1/broadcasts/test/datetime" -d "2024-06-10T15:30:00"

```

2) try to format the json in the Brave browser

