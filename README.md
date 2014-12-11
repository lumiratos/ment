# MENT (Microarray comprEssioN Tools) #

MENT is a set of tools for lossless compression of microarray images. These tools can also be used for other types of images such as medical, RNAi, etc. This set of tools are divided into two categories. One where a bitplane decomposition approach is used and the other one where a binary tree decomposition is used. In what follows, we will describe the set of tools that can be found in MENT:

* **BOSC06** (Bitplane decOmpoSition Compressor 2006) - Lossless compression tool for microarray images introduced by Neves and Pinho in 2006 ([Neves 2006](http://dx.doi.org/10.1109/ICIP.2006.312802)). This tool uses an image-INDEPENDENT context configuration and arithmetic coding.

* **BOSC09** (Bitplane decOmpoSition Compressor 2009) - Lossless compression tool for microarray images introduced by Neves and Pinho in 2009 ([Neves 2009](http://dx.doi.org/10.1109/TMI.2008.929095)). This tool uses an image-DEPENDENT context configuration and arithmetic coding.

* **BOSC09BFS** (Bitplane decOmpoSition Compressor 2009 with Backgroud Foreground Separation) - Tool inspired on method introduced by ([Neves 2009](http://dx.doi.org/10.1109/TMI.2008.929095)) where it was added a segmentation unit as a pre-processing step before the encoding procedure. Results for this tool were first published in [Matos 2010](http://doi.org/10.13140/2.1.3815.5843).

* **BOSC09HC** (Bitplane decOmpoSition Compressor 2009 using Histogram Compaction) - Tool inspired on method introduced by ([Neves 2009](http://dx.doi.org/10.1109/TMI.2008.929095)) where it was added an Histogram Compaction unit in order to remove some redundant bitplanes. This Histogram Compaction is usefull for images that have a reduced number of intensities.

* **BOSC09SBR** (Bitplane decOmpoSition Compressor 2009 using Scalable Bitplane Reduction) - Tool inspired on method introduced by ([Neves 2009](http://dx.doi.org/10.1109/TMI.2008.929095)) where it was added an Scalable Bitplane Reduction unit in order to remove some redundant bitplanes. The Scalable Bitplane Reduction technique was first introduced by [Yoo 1999](http://dx.doi.org/10.1109/ICIP.1999.821655).

* **SBC** (Simple Bitplane Coding) - Tool inspired on one of Kikuchi's work ([Kikuchi 2009](http://dx.doi.org/10.1109/PCS.2009.5167351), [Kikuchi 2012](http://doi.org/10.1587/transfun.E95.A.938)). 

* **BOSC09MixSBC** (Bitplane decOmpoSition Compressor 2009 Mixture with Simple Bitplane Coding) - Tool based on a mixture of finite-context models. In this particular case, we only considered two different models. The first one used by Neves and Pinho ([Neves 2009](http://dx.doi.org/10.1109/TMI.2008.929095)) and the other one based on a Simple Bitplane Coding inspired on Kikuchi's work ([Kikuchi 2009](http://dx.doi.org/10.1109/PCS.2009.5167351), [Kikuchi 2012](http://doi.org/10.1587/transfun.E95.A.938)). 

* **BITTOC** (Binary Tree decomposiTiOn Compressor) - Tool inspired on Chen's work regarding compression of color-quantized images ([Chen 2002](http://dx.doi.org/10.1109/TCSVT.2002.804896)). This tool performance was studied in the context of medical images by Pinho and Neves in 2009 ([Pinho 2009](http://dx.doi.org/10.1109/ICASSP.2009.4959607)) and more recently applied to microarray images ([Matos 2014](http://doi.org/10.13140/2.1.1980.5761)).

* **CmpImgs** (Compare Images) - An image comparasion tool.

# INSTALLATION #
In order to compile the source code, you will need to install a GCC compiler on a Unix platform (Linux or OS X). If you are using Windows, it will be easy to use the pre-compiled binaries that are in folders [win32](https://github.com/lumiratos/ment/blob/master/bin/win32) and [win64](https://github.com/lumiratos/ment/blob/master/bin/win64).

## Linux ##
For Linux users, install the build-essentials package which contains GCC and other utilities in order to be able to compile the source code. To install the build-essentials package type:
<pre>sudo apt-get install build-essential</pre>
After that you only need to type:
<pre>make -f Makefile.linux</pre>
to create the binaries files that can be found at [linux](https://github.com/lumiratos/ment/blob/master/bin/linux).

## OS X ##
For OS X users, it depends on which Xcode version is installed. For the most recent versions, you will need to install the "Command Line Tool" in order to have the "make" utility. It seems that the "Command Line Tools" are not installed by default anymore when you install Xcode. In order to install them, open Xcode, go to Preferences -> Downloads -> Components -> Command Line Tools. This also should install a GCC compiler as well. If you want a recent compiler you can install it using Homebrew by typing the following command in a Terminal:
<pre>brew install gcc48</pre>
After that, we need to make sure that the "CC" variable in the "Makefile.osx" file is linked to the GCC previously installed. The most recent versions of XCode come with a modified version of GCC known as LLVM. This tool was not tested using LLVM so it will probably not work if you try to compile the code using it. In order to generate the binaries just type:
<pre>make -f Makefile.osx</pre>
to create the binaries files that can be found at [osx](https://github.com/lumiratos/ment/blob/master/bin/osx).

## Windows ##
The source code was NOT tested in a Windows enviroment. Nevertheless, you can compile the code using a cross-compiler in a Linux environment after installing the cross-compiler [MinGW-w64](http://mingw-w64.sourceforge.net). After installing MinGW-w64, just type:
<pre>make -f Makefile.win32</pre>
to get the binaries files that can be found [win32](https://github.com/lumiratos/ment/blob/master/bin/win32) for a 32-bits architecture and for the 64-bits architecture just type:
<pre>make -f Makefile.win64</pre> 
to get binaries that can be found at [win64](https://github.com/lumiratos/ment/blob/master/bin/win64). The windows executables were not tested exhaustively so you can find some issues when using them!

# USAGE #
Because MENT contains several tools, we will not list all the paramenters that can be defined by the user. If you execute each compression tool without parameters, it will output all the parameters available. 
<!-- ## Encoding ##
## Decoding ## -->
## Examples ##
In the following, we will show some examples of how to use this tool in a linux environment.

For the case of **BOSC09** we can compile file "1230c1G.pgm.gz" and put the output stream into file "encodedImg.dat" by typing:
<pre>$ BOSC09e -o encodedImg.dat 1230c1G.pgm.gz</pre>
As we can see, **BOSC09** is able to read a given image compressed in gzip.

Now to decode the encoded image, just type:
<pre>$ BOSC09e -o decodedImg.pgm encodedImg.dat</pre>

After the decoding image, you can now perform compare operation using the **CmpImg** tool:
<pre>$ CmpImgs 1230c1G.pgm.gz decodedImg.pgm
L2 Error: 0.000 ; Max Error: 0 ; PSNR: 999.0 dB ; L1 Error: 0.000</pre>

For all the other tool, the paramaters are almost the same. Anywho, if you need help just execute the compression tool without parameters and it will output the parameters available for you.

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

# MICROARRAY DATA SETS #
The microarray data sets were collected from different publicly available sources however, currently some of this data sets
are not available in their original location anymore. We decided then to upload them to an alternative public location that we will indicate next.
<!--
* ApoA1 (32 images | 66.4MB | [download](https://drive.google.com/file/d/0BzRzvourNTdhRl93ZGpwcFd3QnM/edit?usp=sharing))
* Arizona (6 images | 694.9MB | [download](https://drive.google.com/file/d/0BzRzvourNTdhcXlKQ0ViczhzdkU/edit?usp=sharing))
* IBB (44 images | 1.03GB | [download](https://drive.google.com/file/d/0BzRzvourNTdhcUlSWHdHQzBjNkE/edit?usp=sharing))
* ISREC (14 images | 26.7MB | [download](https://drive.google.com/file/d/0BzRzvourNTdhMWFIN2dZQmdSb1k/edit?usp=sharing))
* Omnibus - Low Mode (25 images | 2.5GB | [download](https://drive.google.com/file/d/0BzRzvourNTdhNGVZQ0hPUGZIU2M/edit?usp=sharing))
* Omnibus - High Mode (25 images | 2.5GB | [download ](https://drive.google.com/file/d/0BzRzvourNTdhRHRqaklRUzFSVnc/edit?usp=sharing))
* Stanford (40 images | 396MB | [download](https://drive.google.com/file/d/0BzRzvourNTdhdjZZa1cydDVXZWM/edit?usp=sharing))
* Yeast (109 images | 219MB | [download](https://drive.google.com/file/d/0BzRzvourNTdhWlJpdWZ4azUzNDQ/edit?usp=sharing))
* YuLou (3 images | 40.7MB | [download](https://drive.google.com/file/d/0BzRzvourNTdhSmZ4MFNwMXVxZVk/edit?usp=sharing))
-->
<table align="center">
	<tr> 
          <th width="23%">Data set</th> 
          <th width="20%">Number of images</th>
          <th width="15%">Raw size</th>
          <th width="17%">Download link</th>
          <th width="25%">Original download link</th>
        </tr>
        <tr> 
          <td width="23%" align="center">ApoA1</td> 
          <td width="20%" align="center">32</td>
          <td width="15%" align="center">66.4 MB</td>
          <td width="17%" align="center"><a href="https://drive.google.com/file/d/0BzRzvourNTdhRl93ZGpwcFd3QnM/edit?usp=sharing" target="_blank">download</a></td>
          <td width="25%" align="center"><a href="http://www.stat.berkeley.edu/~terry/zarray/Html/apodata.html" target="_blank">download</a></td>
        </tr>
        <tr> 
          <td width="23%" align="center">Arizona</td> 
          <td width="20%" align="center">6</td>
          <td width="15%" align="center">694.9 MB</td>
          <td width="17%" align="center"><a href="https://drive.google.com/file/d/0BzRzvourNTdhcXlKQ0ViczhzdkU/edit?usp=sharing" target="_blank">download</a></td>
          <td width="25%" align="center"><a href="http://deic.uab.es/~mhernandez/media/imagesets/arizona.tar.bz2" target="_blank">download</a></td>
        </tr>
        <tr> 
          <td width="23%" align="center">IBB</td> 
          <td width="20%" align="center">44</td>
          <td width="15%" align="center">1.03 GB</td>
          <td width="17%" align="center"><a href="https://drive.google.com/file/d/0BzRzvourNTdhcUlSWHdHQzBjNkE/edit?usp=sharing" target="_blank">download</a></td>
          <td width="25%" align="center"><a href="http://deic.uab.es/~mhernandez/media/imagesets/ibb.tar.bz2" target="_blank">download</a></td>
        </tr>
        <tr> 
          <td width="23%" align="center">ISREC</td> 
          <td width="20%" align="center">14</td>
          <td width="15%" align="center">26.7 MB</td>
          <td width="17%" align="center"><a href="https://drive.google.com/file/d/0BzRzvourNTdhMWFIN2dZQmdSb1k/edit?usp=sharing" target="_blank">download</a></td>
          <td width="25%" align="center"><a href="http://ccg.vital-it.ch/DEA/module8/P5_chip_image/images" target="_blank"><strike>dead</strike></a></td>
        </tr>
        <tr> 
          <td width="23%" align="center">Omnibus (low mode)</td> 
          <td width="20%" align="center">25</td>
          <td width="15%" align="center">2.5 GB</td>
          <td width="17%" align="center"><a href="https://drive.google.com/file/d/0BzRzvourNTdhNGVZQ0hPUGZIU2M/edit?usp=sharing" target="_blank">download</a></td>
          <td width="25%" align="center"><a href="http://deic.uab.es/~mhernandez/media/imagesets/omnibus.txt" target="_blank">download</a></td>
        </tr>
        <tr> 
          <td width="23%" align="center">Omnibus (high mode)</td> 
          <td width="20%" align="center">25</td>
          <td width="15%" align="center">2.5 GB</td>
          <td width="17%" align="center"><a href="https://drive.google.com/file/d/0BzRzvourNTdhRHRqaklRUzFSVnc/edit?usp=sharing" target="_blank">download</a></td>
          <td width="25%" align="center"><a href="http://deic.uab.es/~mhernandez/media/imagesets/omnibus.txt" target="_blank">download</a></td>
        </tr>
         <tr> 
          <td width="23%" align="center">Stanford</td> 
          <td width="20%" align="center">40</td>
          <td width="15%" align="center">396 MB</td>
          <td width="17%" align="center"><a href="https://drive.google.com/file/d/0BzRzvourNTdhdjZZa1cydDVXZWM/edit?usp=sharing" target="_blank">download</a></td>
          <td width="25%" align="center"><a href="ftp://smd-ftp.stanford.edu/pub/smd/transfers/Jenny" target="_blank"><strike>dead</strike></a></td>
        </tr>
        <tr> 
          <td width="23%" align="center">Yeast</td> 
          <td width="20%" align="center">109</td>
          <td width="15%" align="center">219 MB</td>
          <td width="17%" align="center"><a href="https://drive.google.com/file/d/0BzRzvourNTdhWlJpdWZ4azUzNDQ/edit?usp=sharing" target="_blank">download</a></td>
          <td width="25%" align="center"><a href="http://genome-www.stanford.edu/cellcycle/data/rawdata/individual.html" target="_blank">download</a></td>
        </tr>
        <tr> 
          <!-- <td width="23%" align="center">[YuLou] [1]</td> -->
          <td width="23%" align="center">YuLou or MicroZip</td>
          <td width="20%" align="center">3</td>
          <td width="15%" align="center">40.7 MB</td>
          <td width="17%" align="center"><a href="https://drive.google.com/file/d/0BzRzvourNTdhSmZ4MFNwMXVxZVk/edit?usp=sharing" target="_blank">download</a></td>
          <td width="25%" align="center"><a href="http://www.cs.ucr.edu/yuluo/MicroZip" target="_blank"><strike>dead</strike></a></td>
        </tr>
</table>
  
# ISSUES #
At the time, there are no relevant issues detected but if you find one please let me know using the [issues link](https://github.com/lumiratos/ment/issues) at GitHub. These tools were tested in a Linux and OS X plataforms. We compile the source code using a cross compiler ([MinGW-w64](http://mingw-w64.sourceforge.net)) to get the windows binaries that are at [win32](https://github.com/lumiratos/ment/blob/master/bin/win32) and [win64](https://github.com/lumiratos/ment/blob/master/bin/win64). We did not had the time to test all the tools in Windows therefore, you can find some issues. 

# COPYRIGHT #
Copyright (c) 2014 Luís M. O. Matos. See [LICENSE.txt](https://github.com/lumiratos/ment/blob/master/LICENSE.txt) for further details.
