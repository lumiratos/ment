# MENT (Microarray comprEssioN Tools) #

MENT is a set of tools for lossless compression of microarray images. These tools can also be used for other types of images such as medical, RNAi, etc. This set of tools are divided into two categories. One where a bitplane decomposition approach is used and the other one where a binary tree decomposition is used. In what follows, we will describe the set of tools that can be found in MENT:

* **BOSC06** (Bitplane decOmpoSition Compressor 2006) - Lossless compression tool for microarray images introduced by Neves and Pinho in 2006 ([Neves 2006](http://dx.doi.org/10.1109/ICIP.2006.312802)). This tool uses an image-INDEPENDENT context configuration and arithmetic coding.

* **BOSC09** (Bitplane decOmpoSition Compressor 2009) - Lossless compression tool for microarray images introduced by Neves and Pinho in 2009 ([Neves 2009](http://dx.doi.org/10.1109/TMI.2008.929095)). This tool uses an image-DEPENDENT context configuration and arithmetic coding.

* **BOSC09BFS** (Bitplane decOmpoSition Compressor 2009 with Backgroud Foreground Separation) - Tool inspired on method introduced by ([Neves 2009](http://dx.doi.org/10.1109/TMI.2008.929095)) where it was added a segmentation unit as a pre-processing step before the encoding procedure. Results for this tool were first published in [Matos 2010](http://doi.org/10.13140/2.1.3815.5843).

* **BOSC09HC** (Bitplane decOmpoSition Compressor 2009 using Histogram Compaction) - Tool inspired on method introduced by ([Neves 2009](http://dx.doi.org/10.1109/TMI.2008.929095)) where it was added an Histogram Compaction unit in order to remove some redundant bitplanes. This Histogram Compaction is usefull for images that have a reduced number of intensities.

* **BOSC09SBR** (Bitplane decOmpoSition Compressor 2009 using Scalable Bitplane Reduction) - Tool inspired on method introduced by ([Neves 2009](http://dx.doi.org/10.1109/TMI.2008.929095)) where it was added an Scalable Bitplane Reduction unit in order to remove some redundant bitplanes. The Scalable Bitplane Reduction technique was first introduced by [Yoo 1999](http://dx.doi.org/10.1109/ICIP.1999.821655).

* **SBC** (Simple Bitplane Coding) - Tool inspired on several Kikuchi's works ([Kikuchi 2009](http://dx.doi.org/10.1109/PCS.2009.5167351), [Kikuchi 2012](http://doi.org/10.1587/transfun.E95.A.938)). 

* **BOSC09MixSBC** (Bitplane decOmpoSition Compressor 2009 Mixture with Simple Bitplane Coding) - Tool based on a mixture of finite-context models. In this particular case, we only considered two different models. The first one used by Neves and Pinho ([Neves 2009](http://dx.doi.org/10.1109/TMI.2008.929095)) and the other one is based on Simple Bitplane Coding inpired on Kikuchi's work ([Kikuchi 2009](http://dx.doi.org/10.1109/PCS.2009.5167351), [Kikuchi 2012](http://doi.org/10.1587/transfun.E95.A.938)). 

* **BITTOC** (Binary Tree decomposiTiOn Compressor) - Tool inspired on Chen's work regarding compression of color-quantized images ([Chen 2002](http://dx.doi.org/10.1109/TCSVT.2002.804896)). This tool performance was studied in the context of medical images by Pinho and Neves in 2009 ([Pinho 2009](http://dx.doi.org/10.1109/ICASSP.2009.4959607)) and more recently applied to microarray images ([Matos 2014](http://doi.org/10.13140/2.1.1980.5761)).

# CITE #
If you use some tool from MENT, please cite the following publications:
* [Luís M. O. Matos](http://sweet.ua.pt/luismatos), [António J. R. Neves](http://sweet.ua.pt/an), [Armando J. Pinho](http://sweet.ua.pt/ap), "Lossy-to-lossless compression of biomedical images based on image decomposition", 
 in Digital Signal Processing, (chapter proposal accepted on December 4, 2014).

* [Luís M. O. Matos](http://sweet.ua.pt/luismatos), [António J. R. Neves](http://sweet.ua.pt/an), [Armando J. Pinho](http://sweet.ua.pt/ap), ["A rate-distortion study on microarray image compression"](http://doi.org/10.13140/2.1.3431.2969), in Proceedings of the 20th Portuguese Conference on Pattern Recognition, [RecPad 2014](http://recpad2014.di.ubi.pt), Covilhã, Portugal, October 2014

* [Luís M. O. Matos](http://sweet.ua.pt/luismatos), [António J. R. Neves](http://sweet.ua.pt/an), [Armando J. Pinho](http://sweet.ua.pt/ap), ["Compression of microarrays images using a binary tree decomposition"](http://doi.org/10.13140/2.1.1980.5761), in Proceedings of the 22nd European Signal Processing Conference, [EUSIPCO 2014](www.eusipco2014.org), Lisbon, Portugal, September 2014.
 
* [Luís M. O. Matos](http://sweet.ua.pt/luismatos), [António J. R. Neves](http://sweet.ua.pt/an), [Armando J. Pinho](http://sweet.ua.pt/ap), ["Compression of DNA microarrays using a mixture of finite-context models"](http://doi.org/10.13140/2.1.1061.8245), in Proceedings of the 18th Portuguese Conference on Pattern Recognition, [RecPad 2012](http://www.isec.pt/recpad2012), Coimbra, Portugal, October 2012.

* [Luís M. O. Matos](http://sweet.ua.pt/luismatos), [António J. R. Neves](http://sweet.ua.pt/an), [Armando J. Pinho](http://sweet.ua.pt/ap), ["Lossy-to-lossless compression of microarrays images using expectation pixel values"](http://doi.org/10.13140/2.1.3553.4403), in Proceedings of the 17th Portuguese Conference on Pattern Recognition, [RecPad 2011](http://paginas.fe.up.pt/~recpad2011), Porto, Portugal, October 2011.

* [Luís M. O. Matos](http://sweet.ua.pt/luismatos), [António J. R. Neves](http://sweet.ua.pt/an), [Armando J. Pinho](http://sweet.ua.pt/ap), ["Lossless compression of microarrays images based on background/foreground separation"](http://doi.org/10.13140/2.1.3815.5843), in Proceedings of the 16th Portuguese Conference on Pattern Recognition, [RecPad 2010](http://recpad2010.utad.pt), Vila Real, Portugal, October 2010.

* [António J. R. Neves](http://sweet.ua.pt/an), [Armando J. Pinho](http://sweet.ua.pt/ap), ["Lossless compression of microarray images using image-dependent finite-context models"](http://dx.doi.org/10.1109/TMI.2008.929095), in [IEEE Transactions on Medical Imaging](http://ieeexplore.ieee.org/xpl/RecentIssue.jsp?punumber=42), volume 28, number 2, pages 194-201, February 2009.

* [António J. R. Neves](http://sweet.ua.pt/an), [Armando J. Pinho](http://sweet.ua.pt/ap), ["Lossless Compression of Microarray Images"](http://dx.doi.org/10.1109/ICIP.2006.31280), in Proceedings of the [IEEE International Conference on Image Processing, ICIP-2006](http://www.icip2006.org), Atlanta, GA, pages 2505-2508, 8-11 October, 2006.
