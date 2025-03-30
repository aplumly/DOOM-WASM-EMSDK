const express = require('express');
const path = require('path');
const app = express();
const PORT = process.env.PORT || 3000;

// Serve static files from the current directory
app.use(express.static(__dirname));

// When the root URL is requested, serve doom.html
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'doom.html'));
});

app.listen(PORT, () => {
  console.log(`Server is running on http://localhost:${PORT}`);
});