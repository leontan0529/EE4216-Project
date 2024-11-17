from PIL import Image
import os
import torch
import torchvision.transforms as transforms

def recognize_face(image_path, known_faces, model):
    """
    Recognizes a face in an image using a pre-trained deep learning model.
    
    Args:
        image_path (str): Path to the image file.
        known_faces (dict): A dictionary mapping class indices to known face names (for debugging purposes).
        model (torch.nn.Module): A pre-trained deep learning model for face recognition (e.g., a CNN).
        
    Returns:
        str: A message indicating whether the person is recognized or if an intruder alert should be triggered.
    """
    
    # Check if the image file exists at the given path
    if not os.path.exists(image_path):
        return "Error: The specified image path does not exist."

    try:
        # Open and preprocess the image
        image = Image.open(image_path).convert('RGB')  # Open the image and ensure it's in RGB format
        
        # Define a transformation pipeline to resize and convert the image to a tensor
        transform = transforms.Compose([
            transforms.Resize((128, 128)),  # Resize the image to 128x128 pixels (standard input size for CNNs)
            transforms.ToTensor(),  # Convert the image to a PyTorch tensor
        ])
        
        # Apply the transformations and add a batch dimension (since PyTorch models expect batch inputs)
        image = transform(image).unsqueeze(0)  # .unsqueeze(0) adds a batch dimension (1, C, H, W)
        
        # Check if the model is loaded
        if model is None:
            return "Error: Model is not loaded."

        # Use the model to predict the class of the image
        with torch.no_grad():  # Disable gradient computation (inference mode)
            output = model(image)  # Get the model's output (raw predictions)
            probabilities = torch.softmax(output, dim=1)  # Convert raw output into probabilities using softmax
            predicted_class_index = torch.argmax(probabilities).item()  # Get the index of the highest probability class
            CNN_OUTPUT = ""  # Initialize the output message

        # If the predicted class is the intruder class (index 2), trigger an alert
        if predicted_class_index == 2:
            CNN_OUTPUT = "Personnel is not recognised. INTRUDER ALERT!"  # Alert for unrecognized person
            return CNN_OUTPUT
        
        # If the predicted class is not the intruder class, return that the user is recognized
        else:
            CNN_OUTPUT = "User is Recognised"  # Recognized user
            return CNN_OUTPUT

    # Catch any exceptions during the recognition process (e.g., image issues, model issues)
    except Exception as e:
        return f"Error during recognition: {e}"  # Return the error message if something goes wrong
