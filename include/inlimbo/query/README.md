# Querying inLimbo

Note that:

1. `SongMap.hpp`: Complete query for the thread safe `SongMap` structure **ONLY**!! (WILL NOT WORK FOR `SongTree`)
2. `Predicates`: Will **ONLY** work for creating predicates to filter the range of iterating over `SongTree`.

Predicates are mainly useful for **command line parsing** (as `SongTree` is NOT thread safe!)

Meanwhile, SongMap queries are useful elsewhere like **UI** and **audio backend**. (where multiple threads are bound be initialized and run.)
