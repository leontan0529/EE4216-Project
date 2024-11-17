import os
import torch
import torch.nn as nn
import torchvision.transforms as transforms
from torchvision import datasets
from torch.utils.data import DataLoader, random_split
import torch.nn.functional as F

def load_model(model_path='face_recognition_cnn.pth'):
    num_classes = 3  # Number of output classes (user_a, user_b, non_user)
    model = FaceRecognitionCNN(num_classes=num_classes)  # Initialize the model
    model.load_state_dict(torch.load(model_path))  # Load model weights from a saved file
    model.eval()  # Set the model to evaluation mode (no gradients calculated)
    return model  # Return the loaded model

def get_data_loaders(data_dir, batch_size=32):
    # Define the image transformations (resizing, flipping, rotation, etc.)
    transform = transforms.Compose([
        transforms.Resize((128, 128)),  # Resize image to 128x128 pixels
        transforms.RandomHorizontalFlip(),  # Randomly flip image horizontally
        transforms.RandomRotation(15),  # Random rotation between -15 and 15 degrees
        transforms.ColorJitter(brightness=0.2, contrast=0.2),  # Randomly adjust brightness and contrast
        transforms.RandomAffine(degrees=10),  # Random affine transformation with slight rotation/translation
        transforms.RandomResizedCrop(size=128, scale=(0.8, 1.0)),  # Random crop with resizing
        transforms.ToTensor(),  # Convert image to tensor
    ])

    # Load dataset using ImageFolder, which expects a directory structure like:
    # /data_dir/training_images/class_name/image.jpg
    full_dataset = datasets.ImageFolder(os.path.join(data_dir, 'training_images'), transform=transform)

    # Split dataset into training and validation sets (80% train, 20% validation)
    train_size = int(0.8 * len(full_dataset))  # 80% of the data for training
    val_size = len(full_dataset) - train_size  # 20% of the data for validation
    train_dataset, val_dataset = random_split(full_dataset, [train_size, val_size])

    # Create DataLoader objects for both training and validation datasets
    train_loader = DataLoader(train_dataset, batch_size=batch_size, shuffle=True)  # Shuffle data during training
    val_loader = DataLoader(val_dataset, batch_size=batch_size, shuffle=False)  # No shuffling for validation

    return train_loader, val_loader, full_dataset  # Return the data loaders and full dataset

class FaceRecognitionCNN(nn.Module):
    def __init__(self, num_classes):
        super(FaceRecognitionCNN, self).__init__()

        # Define the layers of the CNN
        self.conv1 = nn.Conv2d(in_channels=3, out_channels=16, kernel_size=3, stride=1, padding=1)  # First convolution layer
        self.bn1 = nn.BatchNorm2d(16)  # Batch normalization for the first layer
        self.conv2 = nn.Conv2d(in_channels=16, out_channels=32, kernel_size=3, stride=1, padding=1)  # Second convolution layer
        self.bn2 = nn.BatchNorm2d(32)  # Batch normalization for the second layer
        self.pool = nn.MaxPool2d(kernel_size=2, stride=2)  # Max pooling layer
        self.dropout = nn.Dropout(p=0.5)  # Dropout layer to avoid overfitting
        self.fc1 = nn.Linear(32 * 32 * 32, 128)  # Fully connected layer
        self.fc2 = nn.Linear(128, num_classes)  # Final output layer

    def forward(self, x):
        # Forward pass through the network layers
        x = self.pool(F.relu(self.bn1(self.conv1(x))))  # Conv1 -> BatchNorm -> ReLU -> MaxPool
        x = self.pool(F.relu(self.bn2(self.conv2(x))))  # Conv2 -> BatchNorm -> ReLU -> MaxPool
        x = x.view(-1, 32 * 32 * 32)  # Flatten the tensor before the fully connected layers
        x = F.relu(self.fc1(x))  # Fully connected layer 1 with ReLU activation
        x = self.dropout(x)  # Apply dropout to the output of fc1
        x = self.fc2(x)  # Fully connected layer 2 (final output layer)
        return x  # Output the predicted class scores

def train_cnn(data_dir='/Users/constanceow/Desktop/EE4216-Project/docker/machine_learning', num_epochs=15, resume_training=False):
    """Train the CNN model."""
    
    # Get data loaders (train and validation) and the original dataset
    train_loader, val_loader, full_dataset = get_data_loaders(data_dir)

    if train_loader is None or val_loader is None:
        print("Data loaders could not be created. Exiting...")
        return  # Exit if data loaders are not created successfully

    # Initialize the model and optimizer
    num_classes = 3  # Define the number of output classes (user_a, user_b, non_user)
    model = FaceRecognitionCNN(num_classes=num_classes)  # Create an instance of the CNN model
    
    if resume_training:  # Optionally resume training from a saved model
        model.load_state_dict(torch.load('face_recognition_cnn.pth'))
        print("Resuming training from the saved model.")
    
    # Define the loss function (CrossEntropyLoss for classification) and optimizer (Adam)
    criterion = nn.CrossEntropyLoss()  # Loss function for multi-class classification
    optimizer = torch.optim.Adam(model.parameters(), lr=0.0001, weight_decay=1e-5)  # Adam optimizer

    # Training loop with validation monitoring
    for epoch in range(num_epochs):
        model.train()  # Set the model to training mode
        for images, labels in train_loader:  # Loop through training data in batches
            optimizer.zero_grad()  # Zero the gradients from the previous step
            outputs = model(images)  # Forward pass
            loss = criterion(outputs, labels)  # Calculate the loss
            loss.backward()  # Backpropagate the error
            optimizer.step()  # Update the weights

        # Validation phase (evaluating the model without gradients)
        model.eval()  # Set the model to evaluation mode
        val_loss = 0  # Accumulate validation loss
        correct = 0  # Count correct predictions
        total = 0  # Total number of samples

        with torch.no_grad():  # Disable gradient calculations
            for images, labels in val_loader:  # Loop through validation data
                outputs = model(images)  # Forward pass
                val_loss += criterion(outputs, labels).item()  # Accumulate validation loss
                _, predicted = torch.max(outputs.data, 1)  # Get the predicted class
                total += labels.size(0)  # Increment total number of samples
                correct += (predicted == labels).sum().item()  # Count correct predictions

        avg_val_loss = val_loss / len(val_loader)  # Calculate average validation loss

        # Print the training loss, validation loss, and accuracy for this epoch
        print(f'Epoch [{epoch+1}/{num_epochs}], Loss: {loss.item():.4f}, Validation Loss: {avg_val_loss:.4f}, Accuracy: {100 * correct / total:.2f}%')

    # Save the trained model after all epochs
    torch.save(model.state_dict(), 'face_recognition_cnn.pth')
    print("Model training completed and saved as 'face_recognition_cnn.pth'.")
