# Semaphores
## SOI Project
## Requirements

1. **Producers**:  
   - There are `n` producers.  
   - Each producer produces a batch of goods at a time.  
   - The quantity of goods in a batch is random, described by a discrete uniform distribution over the interval `[a, b]`.

2. **Consumers**:  
   - There are `m` consumers.  
   - Each consumer requests a batch of goods at a time.  
   - The quantity of goods in a batch is random, described by a discrete uniform distribution over the interval `[c, d]`.

3. **Warehouse**:  
   - The warehouse has a capacity of `k` units of goods.

4. **Interaction**:  
   - Producers deliver goods to consumers via the warehouse.

5. **Batch Handling**:  
   - Producers and consumers handle only entire batches of goods.  
   - A producer cannot place part of a batch in the warehouse.  
   - A consumer cannot take part of a requested batch from the warehouse.

6. **Timing**:  
   - Production and requests for batches of goods occur no sooner than after a specified time interval (e.g., 1 second).
