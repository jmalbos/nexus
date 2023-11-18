# Getting started

Please follow the instructions of the wiki to install Nexus: https://github.com/next-exp/nexus/wiki/Installing-and-running-nexus

A few additional comments on this particular branch: 
- This version of Nexus requires ROOT. Please make sure it is installed and the environment configured
- Once you have installed all dependencies, please update the directory names in `set_nexus.sh` accordingly and run it every time you open a new terminal for a Nexus session using `source set_nexus.sh`.
- Running Nexus can be done in the two following way (assuming you are in the top level folder of the repository):
    - `./path/to/nexus/build/nexus -b -n <n_events> macros/NDGAr.init.mac` for batch mode
    - `./path/to/nexus/build/nexus -i macros/NDGAr.init.mac` for interactive mode
- If you want to run a batch of simulations, there exists a template macro currently adapted to be used for different initial particle energies, which is called by running `./run_batch.sh`. Please make sure the environment variables are set correctly. An example of an input file is given in `energies.txt`
- Please check `macros/NDGAr.config.mac` for a list of possible parameters that can be set through the macro file.
    - Important: If you want to run optical simulations, `OpticalTrackingAction` must be registered in `NDGAr.init.mac`. For that, the command `/nexus/RegisterTrackingAction OpticalTrackingAction` can simply be commented in, while the command `/nexus/RegisterTrackingAction DefaultTrackingAction` should be commented out