vss replicator
    all set/delete requestes (to the replicator) are sent to memory (needs loopback prevention)
        sent received set/delete requests to the pairs allow mesh networks (but the first replicat (that one received data from a change in the memory), should create an id. This is should be used to prevent networks loops)


    all set/delete requests to memory, sent to the pairs of the replicator. This event is the set/delete origin. In a mesh case, here the pack id should be created to prevent loops.

    memory should be hooked by replicator, allow to request data to pairs if not found in memory. In a mesh case, an pack id should be created to prevent loops. Replicators should send request to their pairs if not found in memory.
    Pairs should responde with their pairs address, in this case, the request is the responsible to manage the search and loop prevention.

    optional (update waves will keep all server updated)
        When a replicator recieves a get request, and do not have the data and need to replicat to its pairs, and receive the respnse, its memory should be udpated (and data should be sen to its pairs? May bee it generates a multiple update wave in the network, once the server with the data should have started an update wave. Reply a request is just a security in the network, for cases when an update wave get a long time to propagate. A reply of request will also happens when an inexistent data is requested to a server, and request multiple inexting data can lead to the network flood, so request reply should be configurable, and default value = false, once is common to receive request for non exisitng data. Clientes will should wait the replication wave).




rede mesh parece ser a melhor opção


memory.hook(set, replicator.set)
#memory.setAlternativeGet(replicator.get)

replicator.set(){
    pack = createPack(arguments, creatteId)
    getPairs().forEach( pair => {
        pair.send(pack)
    })
}

#replicator.get(){
#
#}

replicator.onSetReceived(){
    if alreadyReceived(pack.id) return;

    memory.set(pack.data, do_not_update_replicator);
    getPairs().forEach( pair => {
        if (pair !== this) {
            pair.send(pack);
        }
    });
}