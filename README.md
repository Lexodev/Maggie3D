<h1>Maggie3D project</h1>
<h3>Dynamic C library for drawning textured triangles and quads with Maggie</h3>
<h4>1. How to compile</h4>
<p>To compile the lib you should have a C compiler (SAS/C V6.5X recommanded), you also need an assign on the external libraries, you can find all the includes in the LibInclude repository, just clone it in the same directory where you clone this repo, then add an assign libinclude: to this repo. You should also have vasm in your path, all asm files are compiled with vasm.</p>
<p>The next step is to run the compilation, go into the "src" directory and type "smake", everything should compile. The Maggie3D.library will be created in the src directory, type "smake install" to install the library and the include files in your system. You can now compile other sources from the tests directory.</p>
<p>The "src" directory contains also a small documentation "Maggie3D_doc.txt"</p>
<p>A static version of the library is also present in the "static" directory</p>
