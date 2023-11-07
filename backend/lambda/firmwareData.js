// Load the AWS SDK for Node.js
const AWS = require('aws-sdk');
// Create the DynamoDB service object
const ddb = new AWS.DynamoDB.DocumentClient();
const firmwareTable = process.env.FIRMWARE_TABLE;
exports.handler = async (event) => {
    const bucketName = event.Records[0].s3.bucket.name;
    const fileName = event.Records[0].s3.object.key;
    const s3Time = event.Records[0].eventTime;
    const [file, deviceType, ...fileParts] = fileName.split("_");
    const ver = fileParts.join("_"); // Reconstruct the version string
    const version = ver.replace(/\.[^/.]+$/, "");
    const verNumber = version.replace(/_/g, '.');
    
    console.log(verNumber); 
    const params = {
        TableName: firmwareTable,
        Item: {
             deviceType: deviceType,
             firmwareVersion: verNumber,
             fileName: fileName,
             bucketName: bucketName,
             timestamp:  s3Time,
        },
    };
    
    // Adding the items into the DynamoDB table
    try {
        const data = await ddb.put(params).promise();
    
        return data;
    }
    catch (err) {
        console.log(err);
        const message = `Error inserting object ${firmware_version} into table.`;
        console.log(message);
        throw new Error(message);
    }
};


