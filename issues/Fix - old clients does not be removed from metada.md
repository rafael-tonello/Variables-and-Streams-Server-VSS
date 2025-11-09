# Fix - old clients does not be removed from metada.md
Currently, the database keeps metadata of clients that have not connected for a long time (manly because they changed its ids). This causes a lot of trash data in the database.

## proposal
Detect if the cause is the lack in the bussiness logic or another kind of bug (controller have an device to remove old clients) and fix or implement the solution.

