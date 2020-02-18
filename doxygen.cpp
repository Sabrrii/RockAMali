/*Additional documentation for the generation of the reference page (using doxygen)*/
/**
\mainpage RockAMali software suite
*
* \image html logo.png
*
* table of content:
* \li \ref sectionCodeHistory
* \li \ref sectionCommandLine
* \li \ref sectionRunCommand
* \li \ref sectionDocumentation
* \li \ref sectionDoxygenSyntax
*
* \section sectionCodeHistory Modifications
*

    S. Coudert 26/08/2019
     - doxygen for entire software suite

* \section sectionCommandLine command line options
*
* \note all process that have processing class might have \c --use-GPU CLI option if \c DO_GPU is set in \c Makefile
*
* - process
* \verbinclude "process_help.output"
*
* - process (sequential version)
* \verbinclude "process_sequential_help.output"
*
* - receive (see especially \c -c and \c --do-check CLI options)
* \verbinclude "receive_help.output"
*
* - send
* \verbinclude "send_help.output"
*
* - store (old process)
* \verbinclude "store_help.output"
*
* \section sectionCompile compiling processes
*
* - many current processes\n
*   \c make \c all
*
* - single one, e.g. process\n
*   \c make \c process
*   \see \c Makefile and \c _info.txt
*
* - clear images\n
*   \c make \c clear
*
* - clean all\n
*   \c make \c clean
*
* \section sectionRunCommand running processes
*
* - process* on their own\n
*   \c make \c process_run
*   \see \c Makefile and \c _info.txt
*
* - send and receive\n
*   \c make \c receive_run
*   \c make \c send_run
*   \see \c Makefile and \c _info.txt
*
* \section sectionDocumentation documentation outline
* This is the reference documentation of 
* <a href="http://wiki.ganil.fr/gap/">RockAMali</a> software suite, 
* from the <a href="http://www.ganil-spiral2.eu">GANIL</a>.
*
* \par Programs:
*
* The main functions are in several files (one source file for each process)
* - <a href="process_8cpp.html">process</a>
* - <a href="process__sequential_8cpp.html">process sequential</a>
* - <a href="receive_8cpp.html">receive</a>
* - <a href="send_8cpp.html">send</a>
*
* \par Creation:
*
* This documentation has been automatically generated from the sources,
* (and especially <a href="doxygen_8cpp.html">doxygen.cpp</a> for this main page -no C++ code-)
* using the tool <a href="http://www.doxygen.org">doxygen</a>. It should be readed as HTML, LaTex and man page.\n
* It contains both
* \li a detailed description of all classes and functions
* \li TODO: a user guide (cf. \ref pages.html "related pages") \see \c _info.txt
*
* that as been documented within the sources.
*
* \par Additional needed libraries:
*
* \li \c CImg : containers and CLI (and its basic operations) -CLI: Command Line Interface-
* \li \c BOOST::ASIO : (ethernet UDP)
* \li \c BOOST::compute : (GPU with OpenCL)
*
* \par Optional libraries:
*
* \li \c -NetCDF: Network Common Data Form-
* \li \c CImg.Tool : data storage (.nc)
* \li \c NetCDF.Tool : parameter file (PIF) (.cdl -> .nc) -PIF: Parameter Interface File-
*
* \par Optional tools:
*
* \li \c NetCDF: ncgen, ncdump
* \li \c NCO: ncrename, ncap2 (NetCDF operator)
* \li \c ncview: NetCDF visualisation
*
* \section sectionDoxygenSyntax make documentation using Doxygen syntax
* Each function in the source code should be commented using \b doxygen \b syntax in the same file.
* The documentation need to be written before the function.
* The basic syntax is presented in this part.
* \verbinclude "doxygen.example1.txt"
*
* Two kind of comments are needed for \b declaration and \b explanation \b parts of the function:
* Standart documentation should the following (\b sample of code documentation):
* \verbinclude "doxygen.example2.txt"
*
* In both declaration and explanation part, \b writting and \b highlithing syntax can be the following:\n\n
*
* \li \c \\n a new line
* \li \c \\li a list (dot list)
*
* \li \c \\b bold style
* \li \c \\c code style
* \li \c \\e enhanced style (italic)
*
* For making \b shortcut please use:\n
* \li \c \\see to make a shortcut to a related function or variable
* \li \c \\link to make a shortcut to a file or a function
* \note this keyword needs to be closed using \c \\end*
*
* \li \c \\todo to add a thing to do in the list of <a href="todo.html">ToDo</a> for the whole program
*
* In explanation part, \b paragraph style can be the following:\n
* \li \c \\code for an example of the function use
* \li \c \\note to add a few notes
* \li \c \\attention for SOMETHING NOT FULLY DEFINED YET
* \li \c \\warning to give a few warning on the function
* \note these keywords need to be closed using \c \\end*
*
* \verbinclude "doxygen.example3.txt"
*
* Many other keywords are defined, so please read the documentation of <a href="http://www.doxygen.org/commands.html">doxygen</a>.
*
**/

/**
\page pagePerfs Performances for RockAMali
* table of content:
* \li \ref sectionStore
* \li \ref sectionNetwork
*
* \section sectionStore storing performances
*
* \note clean image files before running, as if files are overrited, it hield to smaller performances, e.g. 120MB/s to 80MB/s, so please remove image files !
*
* \verbinclude "process_perfs.dat"
* 
* \image html store_plot.png "data rate vs frame size"
*
* \section sectionNetwork ethernet performances
*
* \image html network_3Dplot.png "data rate vs wait time and frame size"
*
**/

