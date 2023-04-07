<<<<<<< HEAD
# 3313-Group
=======
<br />
<div align="center">
  <a href="#">
    <img src="./src/assets/projectLogo.png" alt="Logo" width="80">
  </a>

  <p align="center">
    Made using ReactJS and Tailwind!
    <br />
    <br />
    <a target="_blank" href="https://frogjack.netlify.app/">View Demo</a>
    ·
    <a href="https://github.com/erickctra/frog-jack/issues">Report Bug</a>
    ·
    <a href="https://github.com/erickctra/frog-jack/issues">Request Feature</a>
  </p>
</div>

## Setting up Docker Environment

1. Make sure you've installed Docker and all dependencies; get up to the end of Part 3
2. Save the Dockerfile in the Github to where you want your container to be
3. Open terminal and `cd` into the folder with the Dockerfile
4. Run the following commands to create and start the container:

```
docker build -t se3313 .
docker container create -i -t --name 3313-project -p 3000:3000 -p 2200:22 se3313
docker start 3313-project
docker exec -it 3313-project /bin/bash
```

5. Open VSCode; install `Microsoft Remote - SSH`
6. Hit `Ctrl+Shift+P` and type in `Connect to Host`. Select the first option.
7. Type in `student@localhost:2200`
8. Click create a new host
9. Click `localhost`
10. Type in `password` for the password
11. Click Open Folder and open the main folder
12. you figure out the rest

## About The Project

![alt text](https://raw.githubusercontent.com/erickctra/frog-jack/master/src/assets/screenshot.png)

### Built With

Here are the libraries that I used in this project

- [React.js](https://reactjs.org/)
- [TailwindCss](https://tailwindcss.com/)

<!-- GETTING STARTED -->

## Getting Started

This is an example of how you may give instructions on setting up your project locally.
To get a local copy up and running follow these simple example steps.

### Prerequisites

To run this project you will need to have Node.js installed

- [Node](https://nodejs.org/en/)

### Installation

Follow the instructions to clone this project

2. Clone the repo
   ```sh
   git clone https://github.com/erickctra/frog-jack.git
   ```
3. Install NPM packages
   ```sh
   npm install
   ```
4. Run
   ```js
   npm run dev
   ```

<p align="right">(<a href="#top">back to top</a>)</p>
>>>>>>> origin/master
