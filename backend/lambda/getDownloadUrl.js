const AWS = require('aws-sdk');
const semver = require('semver');

// AWS DynamoDB DocumentClient instance
const dynamoDB = new AWS.DynamoDB.DocumentClient();

// Constants for DynamoDB table and device type
const firmwareTable = process.env.FIRMWARE_TABLE;
const DEVICE_TYPE = 'esp32';

// Lambda handler function
exports.handler = async (event) => {
  try {
    console.log(event);

    // Validate the rawVersion parameter
    const versionValid = validateVersionParam(event);

    if (!versionValid) {
      console.log("Failed to Pass Query Parameter");
      return response({
        statusCode: 400,
        data: { error: "Invalid Parameter" },
      });
    }

    const currentVersion = event.queryStringParameters.rawVersion;
    console.log(currentVersion);

    // Read data from DynamoDB
    const dbResult = await readDataFromDb();

    // Check for a firmware update
    const needsUpdate = checkForUpdate(dbResult, currentVersion);

    if (needsUpdate) {
      const url = getFirmwareUrl(dbResult);
      return response({
        statusCode: 200,
        data: { Response: url },
      });
    } else {
      return response({
        statusCode: 200,
        data: { Response: "Device up to date" },
      });
    }
  } catch (error) {
    console.log(error);
    return response({
      statusCode: 503,
      data: { error: "An error occurred. Try again later.", stack: { error } },
    });
  }
};

// Read data from DynamoDB
const readDataFromDb = async () => {
  const params = {
    TableName: firmwareTable,
    KeyConditionExpression: "#deviceType = :deviceType",
    ExpressionAttributeNames: { "#deviceType": "deviceType" },
    ExpressionAttributeValues: { ":deviceType": DEVICE_TYPE },
  };

  const result = await dynamoDB.query(params).promise();
  return result;
};

// Check if an update is required
const checkForUpdate = (result, currentVersion) => {
  const firmwareVersion = result.Items[0].firmwareVersion;
  return semver.gt(firmwareVersion, currentVersion);
};

// Get the download URL from the database result
const getFirmwareUrl = (result) => {
  const bucketName = result.Items[0].bucketName;
  const fileName = result.Items[0].fileName;
  const url = `https://${bucketName}.s3.amazonaws.com/${fileName}`;
  return url;
};

// Validate the rawVersion parameter
const validateVersionParam = (event) => {
  const params = event.queryStringParameters;

  if (params && params.rawVersion) {
    const rawVersion = params.rawVersion;

    if (typeof rawVersion === "string" && /^v[0-9].[0-9].[0-9]/.test(rawVersion)) {
      console.log('validated');
      return true;
    }
  }

  return false;
};

// Build the response object
const response = ({ statusCode, data, header = {} }) => {
  const headers = {
    "Access-Control-Allow-Origin": "*",
    "Content-Type": "application/json",
    ...header,
  };
  const body = JSON.stringify(data);

  return {
    statusCode,
    headers,
    body,
  };
};