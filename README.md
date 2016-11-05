Sound Measurement Application

Requirements for compilation (ubuntu):
 - cmake
 - g++
 - libpulse-dev
 - libgtkmm-3.0-dev

Requirements for document creation (ubuntu):
 - pandoc
 - texlive
 - texlive-fonts-recommended
 - texlive-lang-european

Compilation:
 - mkdir build && cd build
 - cmake ..
 - make

Document creation:
 - cd doc
 - ./makedoc.sh

