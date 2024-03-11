## Building the Project for Visual Studio (Windows) as .dll

```bash
# Initialize a new Git repository
git init

# Add the remote repository
git remote add origin https://github.com/Azzinoth/UnityPointCloudPlugin

# Pull the contents of the remote repository
git pull origin master

# Initialize and update submodules
git submodule update --init --recursive

# Generate the build files using CMake
# CMake should be added to a PATH
# if not then use CMAKE GUI
cmake CMakeLists.txt
```