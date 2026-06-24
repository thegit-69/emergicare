# --- Stage 1: Build the React Application ---
# Use an official Node.js image to create the build artifacts
FROM node:20-alpine AS build

# Set the working directory inside the container
WORKDIR /app

# Copy package.json and package-lock.json to leverage Docker's layer caching
COPY package*.json ./

# Install project dependencies
RUN npm install

# Copy the rest of the application source code
COPY . .

# Build the application for production
RUN npm run build


# --- Stage 2: Serve the Application with Nginx ---
# Use a lightweight, official Nginx image for the production server
FROM nginx:1.25-alpine

# Copy the static build files from the 'build' stage to Nginx's web server directory
COPY --from=build /app/dist /usr/share/nginx/html

# Nginx will automatically serve the index.html file in this directory
# Expose port 80 for the Nginx server
EXPOSE 80

# The default command for the nginx image is to start the server, so we don't need a CMD
