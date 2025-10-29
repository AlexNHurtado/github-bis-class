// This code runs on a separate server (e.g., Node.js server), not the ESP32!

const express = require('express');
const mongoose = require('mongoose');
const dotenv = require('dotenv');

// Load environment variables from .env file (e.g., MONGO_URI, PORT)
dotenv.config(); 

const app = express();
const PORT = process.env.PORT || 3000;
const MONGO_URI = process.env.MONGO_URI || 'mongodb://localhost:27017/esp32_iot'; 
// --- REPLACE THE MONGO_URI WITH YOUR ACTUAL CONNECTION STRING ---

// Middleware
app.use(express.json()); // Used to parse the JSON data sent by the ESP32

// -----------------------------------------------------------------
// 1. Mongoose Model (Defines the structure of the data in MongoDB)
// -----------------------------------------------------------------
const sensorDataSchema = new mongoose.Schema({
    device_id: {
        type: String,
        required: true,
        trim: true
    },
    temperature: {
        type: Number,
        required: true
    },
    // Add a timestamp automatically on creation
    timestamp: {
        type: Date,
        default: Date.now
    }
});

const SensorData = mongoose.model('SensorData', sensorDataSchema);


// -----------------------------------------------------------------
// 2. Database Connection
// -----------------------------------------------------------------
mongoose.connect(MONGO_URI)
    .then(() => console.log('Successfully connected to MongoDB.'))
    .catch(err => {
        console.error('MongoDB connection error:', err);
        // Exit process on fatal error
        process.exit(1); 
    });


// -----------------------------------------------------------------
// 3. API Endpoints (Matches the ESP32's Model expectations)
// -----------------------------------------------------------------

/**
 * Endpoint to save new sensor data (POST /api/v1/data/save)
 * Expected body from ESP32: { "device_id": "ESP32_001", "temperature": 26.5 }
 */
app.post('/api/v1/data/save', async (req, res) => {
    // Basic validation
    const { device_id, temperature } = req.body;
    if (!device_id || typeof temperature !== 'number') {
        return res.status(400).json({ error: 'Missing or invalid device_id or temperature.' });
    }

    try {
        // Create and save the new document
        const newData = new SensorData({ device_id, temperature });
        await newData.save();
        
        // Respond to the ESP32 with 201 Created
        console.log(`[SAVE] Data received and stored: ${device_id} @ ${temperature}°C`);
        res.status(201).json({ message: 'Data saved successfully.' });

    } catch (error) {
        console.error('[SAVE] Database write error:', error);
        res.status(500).json({ error: 'Failed to save data to database.' });
    }
});


/**
 * Endpoint to retrieve the latest sensor data (GET /api/v1/data/latest)
 * Expected query from ESP32: ?device=ESP32_001
 */
app.get('/api/v1/data/latest', async (req, res) => {
    // Get device ID from query parameters
    const deviceId = req.query.device;

    if (!deviceId) {
        return res.status(400).json({ error: 'Missing device ID query parameter.' });
    }

    try {
        // Find the single latest record for the specified device ID
        const latestReading = await SensorData.findOne({ device_id: deviceId })
            .sort({ timestamp: -1 }) // Sort by newest first
            .limit(1); // Get only the first (latest) one

        if (latestReading) {
            // Send back the temperature data (and timestamp, if desired)
            console.log(`[FETCH] Sent latest reading for ${deviceId}`);
            res.status(200).json({ 
                temperature: latestReading.temperature,
                timestamp: latestReading.timestamp 
            });
        } else {
            console.log(`[FETCH] No data found for ${deviceId}`);
            res.status(404).json({ error: 'No data found for this device.' });
        }

    } catch (error) {
        console.error('[FETCH] Database read error:', error);
        res.status(500).json({ error: 'Failed to retrieve data from database.' });
    }
});


// -----------------------------------------------------------------
// 4. Start Server
// -----------------------------------------------------------------
app.listen(PORT, () => {
    console.log(`MongoDB Proxy API running on http://localhost:${PORT}`);
    // This is the address you would need to expose and use in your ESP32 code!
});
