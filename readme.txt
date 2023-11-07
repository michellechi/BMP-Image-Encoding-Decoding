How much smaller (in %) is the compressed image?
On my Mac I was able to achieve around a 5 to 30 percent compression ratio. However, on my virtual machine running Ubuntu, the compression process actually expanded the image.
I think this was largely due to the fact that the operating systems treated the priority queue in different manners, thus causing the compression to be different. 
Both of them handle the compression and decompression just fine, but the efficiency of the compression is different.

How long does it take to compress and decompress it? Any ideas how to parallelize it?
It takes about 220 ms to compress the file and 175 ms to decompress it. The easiest parallelizing savings would be to parallelize the grayscaling of the image.
Another place that could be parallelized is the counting of the frequencies of the individual rgb values.

Where? What is the time you win?
In my code I was able to parallelize the grayscaling of the image. This saved me about 10% of the total time depending on the size of the image. The actual time saved was not
impacted heavily because the grayscaling is a small part of the overall time. The encoding takes far longer.