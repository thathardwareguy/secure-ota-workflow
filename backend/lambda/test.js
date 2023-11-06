
const fileName = "firmware_esp32_v1_4_6.bin"; // Your file name

const [file, deviceType, ...fileParts] = fileName.split("_");
const ver = fileParts.join("_"); // Reconstruct the version string
const version = ver.replace(/\.[^/.]+$/, "");
const verNumber = version.replace(/_/g, '.');

console.log(verNumber); 