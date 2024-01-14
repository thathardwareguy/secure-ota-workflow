# Serverless CICD and OTA flow for IoT devices with AWS and PlatformIO 

How to build your firmware continuously in the cloud and send to your devices automatically.

Blog post with details: https://embeddedartistry.com/blog/2024/01/15/exploring-serverless-ci-cd-for-embedded-devices/

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

	![Serverless Application](./images/serverless-provider-settings.PNG)
* For subsequent deployments:
    * `serverless deploy`
* Lambda Functions Deployment Output
    ![Serverless Deploy Output](./images/aws-serverless-output.PNG)
* Copy and save the endpoint 


