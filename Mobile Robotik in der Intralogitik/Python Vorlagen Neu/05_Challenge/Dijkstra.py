def dijkstra(startnode,endnode):
    NB = {"a": {"b": 2, "d": 5, "f": 6},
          "b": {"c": 3, "e": 2},
          "c": {"g": 8, "e": 1},
          "d": {"e": 1, "h": 10},
          "e": {"g": 10, "h": 10},
          "f": {"d": 2, "i": 4},
          "g": {"j": 8},
          "h": {"j": 6, "k": 6},
          "i": {"h": 7, "l": 4},
          "j": {"n": 5, "k": 7},
          "k": {"i": 7, "n": 9},
          "l": {"m": 9},
          "m": {"k": 1},
          "n": {"o": 1},
          "o": {},
          }
    
    NB_Test = {"a": {"b": 10},
          "b": {"c": 1},
          "c": {"e": 3},
          "d": {"b": 4},
          "e": {"f": 22, "d": 10},
          "f": {}
          }

    MK = [startnode]
    D = {"a": 99999, "b": 99999, "c": 99999, "d": 99999, "e": 99999, "f": 99999, "g": 99999, "h": 99999, "i": 99999, "j": 99999, "k": 99999, "l": 99999, "m": 99999, "n": 99999, "o": 99999, "p": 99999, "q": 99999}
    R = {"a": [], "b": [], "c": [], "d": [], "e": [], "f": [], "g": [], "h": [], "i": [], "j": [], "k": [], "l": [], "m": [], "n": [], "o": [], "p": [], "q": []}
    

    D[startnode] = 0

    while len(MK) != 0:

        # 1
        dis_start = 999999
        for node in MK:
            dis_start_node = D[node]
            if dis_start_node < dis_start:
                new_node = node
                dis_start = dis_start_node
        current_node = new_node

        # 2
        for neighbour in NB[current_node]:
            if D[neighbour] > D[current_node] + NB[current_node][neighbour]:
                D[neighbour] = D[current_node] + NB[current_node][neighbour]
                R[neighbour].clear()
                R[neighbour].append(current_node)                
                MK.append(neighbour)
                R[neighbour].extend(R[current_node])

        # 3
        MK.remove(current_node)

    for node in R:
        R[node].insert(0,node)
    route = R[endnode]
    route.reverse()

    #print(D)
    #print(R)
    return route