# _Sample project_

(See the README.md file in the upper level 'examples' directory for more information about examples.)

This is the simplest buildable example. The example is used by command `idf.py create-project`
that copies the project to user specified path and set it's name. For more information follow the [docs page](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#start-a-new-project)



## How to use example
We encourage the users to use the example as a template for the new projects.
A recommended way is to follow the instructions on a [docs page](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#start-a-new-project).

## Example folder contents

The project **sample_project** contains one source file in C language [main.c](main/main.c). The file is located in folder [main](main).

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt`
files that provide set of directives and instructions describing the project's source files and targets
(executable, library, or both). 

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   └── main.c
└── README.md                  This is the file you are currently reading
```
Additionally, the sample project contains Makefile and component.mk files, used for the legacy Make based build system. 
They are not used or needed when building with CMake and idf.py.

# Serverless CICD and OTA flow for IoT devices with AWS and PlatformIO 

How to build your firmware continuously in the cloud and sent to your devices automatically.

Blog post with details: 

### Install all dependencies

* Clone this repository.
* Run npm install:
    * `npm install`

* Serverless Framework Setup :
  * Create a serverless account at https://app.serverless.com 
  * For this project, Github account was used to sign in.

* AWS Account Setup :
  * Create a AWS account at https://aws.amazon.com  
  * Create an IAM user and retrieve the Secret Access Key and Key ID.

* Github Actions Setup :
  * Create a new repository on Github.
  * Navigate to the main page of the repository and click on settings.
  * In the left sidebar, click Secrets and click New repository secret.
  * Type the secret names has shown in the firmwarebuild.yml and save.

### Deploy lambda Functions

* Navigate into the backend directory.
    * `cd backend`
* For the first time, create an application in your org in Serverless portal:
    * `serverless`
![Serverless Application](./images/serverless-dashboard.PNG)
* For subsequent deployments:
    * `serverless deploy`
* Lambda Functions Deployment Output
![Serverless Deploy Output](./images/serverless-deploy-output.PNG)
* Copy and save the endpoint 


### Upload firmware with PlatfomIO

I recommend installing the Visual Studio Code (VSCode) IDE and the PlatformIO plugin to get started using it. Just follow the step on the link below: 

https://platformio.org/platformio-ide

To deploy to the board you can use the “Build” and “Upload” buttons on PlatformIO Toolbar.


### References

*  https://dzone.com/articles/how-to-approach-ota-updates-for-iot
*  https://github.com/alvarowolfx/gcloud-ota-arduino-update 
*  https://docs.aws.amazon.com 
*  https://docs.platformio.org/en/latest/integration/ci/github-actions.html 
