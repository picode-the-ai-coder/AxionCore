"""
AxionAI - AI Model Integration for Blockchain

Enables deployment and execution of machine learning models on Axion blockchain.

Features:
- Model training and validation
- On-chain inference
- Model versioning
- Fee distribution for AI services

Installation:
    pip install numpy scikit-learn tensorflow torch pandas

Example:
    from axion_ai import AxionAIModel, ModelType
    
    # Create and train model
    model = AxionAIModel(ModelType.NEURAL_NETWORK)
    model.train(X_train, y_train, epochs=100)
    
    # Deploy to blockchain
    contract_addr = model.deploy_to_blockchain()
    
    # Run inference
    prediction = model.predict(X_test)
"""

from typing import Dict, List, Any, Tuple, Optional
from dataclasses import dataclass
from enum import Enum
import json
import hashlib
import pickle
import numpy as np
from abc import ABC, abstractmethod

# ====================================================================
# Model Types
# ====================================================================

class ModelType(Enum):
    """Supported model types"""
    LINEAR_REGRESSION = "linear_regression"
    LOGISTIC_REGRESSION = "logistic_regression"
    RANDOM_FOREST = "random_forest"
    NEURAL_NETWORK = "neural_network"
    SVM = "svm"
    GRADIENT_BOOSTING = "gradient_boosting"
    CLUSTERING = "clustering"
    CUSTOM = "custom"

# ====================================================================
# Model Metadata
# ====================================================================

@dataclass
class ModelMetadata:
    """Model information"""
    name: str
    version: str
    model_type: ModelType
    created_at: int
    updated_at: int
    creator: str
    input_shape: Tuple[int, ...]
    output_shape: Tuple[int, ...]
    accuracy: Optional[float] = None
    loss: Optional[float] = None
    parameters: Dict[str, Any] = None

# ====================================================================
# AxionAI Model Base Class
# ====================================================================

class AxionAIModel(ABC):
    """
    Base class for AI models on Axion blockchain
    
    Features:
    - Training and validation
    - Serialization
    - Blockchain deployment
    - Inference
    """
    
    def __init__(self, model_type: ModelType, name: str = "AxionModel"):
        """
        Initialize AI model
        
        Args:
            model_type: Type of model
            name: Model name
        """
        self.model_type = model_type
        self.name = name
        self.model = None
        self.metadata = None
        self.scaler = None
        self._trained = False
    
    @abstractmethod
    def train(self, X_train, y_train, **kwargs):
        """Train the model"""
        pass
    
    @abstractmethod
    def predict(self, X):
        """Run inference"""
        pass
    
    def validate(self, X_test, y_test) -> Dict[str, float]:
        """
        Validate model on test data
        
        Returns:
            Metrics (accuracy, precision, recall, etc.)
        """
        predictions = self.predict(X_test)
        
        if self.model_type in [
            ModelType.LINEAR_REGRESSION,
            ModelType.NEURAL_NETWORK
        ]:
            # Regression metrics
            from sklearn.metrics import mean_squared_error, r2_score
            mse = mean_squared_error(y_test, predictions)
            r2 = r2_score(y_test, predictions)
            return {
                "mse": float(mse),
                "rmse": float(np.sqrt(mse)),
                "r2": float(r2)
            }
        else:
            # Classification metrics
            from sklearn.metrics import (
                accuracy_score, precision_score, recall_score, f1_score
            )
            acc = accuracy_score(y_test, predictions)
            prec = precision_score(y_test, predictions, average='weighted')
            rec = recall_score(y_test, predictions, average='weighted')
            f1 = f1_score(y_test, predictions, average='weighted')
            
            return {
                "accuracy": float(acc),
                "precision": float(prec),
                "recall": float(rec),
                "f1": float(f1)
            }
    
    def serialize(self) -> bytes:
        """Serialize model to bytes"""
        return pickle.dumps({
            'model': self.model,
            'scaler': self.scaler,
            'metadata': self.metadata
        })
    
    def deserialize(self, data: bytes) -> bool:
        """Deserialize model from bytes"""
        try:
            obj = pickle.loads(data)
            self.model = obj['model']
            self.scaler = obj['scaler']
            self.metadata = obj['metadata']
            self._trained = True
            return True
        except Exception as e:
            print(f"Deserialization failed: {e}")
            return False
    
    def get_model_hash(self) -> str:
        """Get SHA256 hash of model"""
        model_bytes = self.serialize()
        return hashlib.sha256(model_bytes).hexdigest()
    
    def deploy_to_blockchain(self, **kwargs) -> Dict[str, Any]:
        """
        Deploy trained model to Axion blockchain
        
        Returns:
            Deployment information
        
        Example:
            result = model.deploy_to_blockchain()
            print(result['contract_address'])
        """
        if not self._trained:
            raise RuntimeError("Model not trained yet")
        
        model_bytes = self.serialize()
        model_hash = self.get_model_hash()
        
        # This would be done by the blockchain
        return {
            "success": True,
            "model_hash": model_hash,
            "model_size": len(model_bytes),
            "model_type": self.model_type.value,
            "deployed_at": "block_number_here",
            "contract_address": f"0x{model_hash[:40]}"
        }
    
    def run_inference_on_chain(self,
                              contract_address: str,
                              input_data: np.ndarray) -> Dict[str, Any]:
        """
        Run inference on deployed blockchain model
        
        Args:
            contract_address: Deployed model contract address
            input_data: Input for inference
        
        Returns:
            Prediction result
        """
        # This would interact with the blockchain contract
        prediction = self.predict(input_data)
        
        return {
            "contract": contract_address,
            "prediction": prediction.tolist() if isinstance(prediction, np.ndarray) else prediction,
            "model_hash": self.get_model_hash()
        }

# ====================================================================
# Concrete Model Implementations
# ====================================================================

class NeuralNetworkModel(AxionAIModel):
    """Neural Network using TensorFlow/Keras"""
    
    def __init__(self, name: str = "NeuralNetwork"):
        super().__init__(ModelType.NEURAL_NETWORK, name)
    
    def train(self, X_train, y_train, epochs: int = 100, batch_size: int = 32):
        """
        Train neural network
        
        Args:
            X_train: Training features
            y_train: Training labels
            epochs: Number of epochs
            batch_size: Batch size
        """
        try:
            import tensorflow as tf
            from sklearn.preprocessing import StandardScaler
        except ImportError:
            raise ImportError("Install TensorFlow: pip install tensorflow")
        
        # Normalize features
        self.scaler = StandardScaler()
        X_train_scaled = self.scaler.fit_transform(X_train)
        
        # Build model
        input_shape = X_train.shape[1]
        self.model = tf.keras.Sequential([
            tf.keras.layers.Dense(128, activation='relu', input_shape=(input_shape,)),
            tf.keras.layers.Dropout(0.2),
            tf.keras.layers.Dense(64, activation='relu'),
            tf.keras.layers.Dropout(0.2),
            tf.keras.layers.Dense(32, activation='relu'),
            tf.keras.layers.Dense(1)
        ])
        
        self.model.compile(optimizer='adam', loss='mse', metrics=['mae'])
        
        # Train
        history = self.model.fit(
            X_train_scaled, y_train,
            epochs=epochs,
            batch_size=batch_size,
            verbose=0
        )
        
        self._trained = True
        return history
    
    def predict(self, X):
        """Run inference"""
        if self.model is None:
            raise RuntimeError("Model not trained")
        
        X_scaled = self.scaler.transform(X)
        return self.model.predict(X_scaled, verbose=0)

class RandomForestModel(AxionAIModel):
    """Random Forest classifier/regressor"""
    
    def __init__(self, is_classifier: bool = True, name: str = "RandomForest"):
        super().__init__(ModelType.RANDOM_FOREST, name)
        self.is_classifier = is_classifier
    
    def train(self, X_train, y_train, n_estimators: int = 100, max_depth: int = 20):
        """
        Train random forest
        
        Args:
            X_train: Training features
            y_train: Training labels
            n_estimators: Number of trees
            max_depth: Maximum tree depth
        """
        try:
            from sklearn.ensemble import RandomForestClassifier, RandomForestRegressor
        except ImportError:
            raise ImportError("Install scikit-learn: pip install scikit-learn")
        
        if self.is_classifier:
            self.model = RandomForestClassifier(
                n_estimators=n_estimators,
                max_depth=max_depth
            )
        else:
            self.model = RandomForestRegressor(
                n_estimators=n_estimators,
                max_depth=max_depth
            )
        
        self.model.fit(X_train, y_train)
        self._trained = True
    
    def predict(self, X):
        """Run inference"""
        if self.model is None:
            raise RuntimeError("Model not trained")
        return self.model.predict(X)

class GradientBoostingModel(AxionAIModel):
    """Gradient Boosting model"""
    
    def __init__(self, name: str = "GradientBoosting"):
        super().__init__(ModelType.GRADIENT_BOOSTING, name)
    
    def train(self, X_train, y_train, n_estimators: int = 100, learning_rate: float = 0.1):
        """Train gradient boosting model"""
        try:
            from sklearn.ensemble import GradientBoostingRegressor
        except ImportError:
            raise ImportError("Install scikit-learn: pip install scikit-learn")
        
        self.model = GradientBoostingRegressor(
            n_estimators=n_estimators,
            learning_rate=learning_rate
        )
        
        self.model.fit(X_train, y_train)
        self._trained = True
    
    def predict(self, X):
        """Run inference"""
        if self.model is None:
            raise RuntimeError("Model not trained")
        return self.model.predict(X)

# ====================================================================
# Model Registry (On-chain)
# ====================================================================

class ModelRegistry:
    """
    Registry of deployed AI models on Axion blockchain
    
    Tracks:
    - Model versions
    - Deployment status
    - Performance metrics
    - Usage statistics
    """
    
    def __init__(self):
        self.models: Dict[str, Dict[str, Any]] = {}
    
    def register_model(self,
                      model_hash: str,
                      metadata: ModelMetadata,
                      contract_address: str):
        """Register deployed model"""
        self.models[model_hash] = {
            "metadata": metadata,
            "contract_address": contract_address,
            "deployed_blocks": 0,
            "inference_count": 0,
            "total_fees": 0
        }
    
    def get_model(self, model_hash: str) -> Optional[Dict]:
        """Get model info"""
        return self.models.get(model_hash)
    
    def list_models(self) -> List[str]:
        """List all registered models"""
        return list(self.models.keys())
    
    def get_model_stats(self, model_hash: str) -> Dict[str, Any]:
        """Get model statistics"""
        model = self.get_model(model_hash)
        if not model:
            return {}
        
        return {
            "inference_count": model["inference_count"],
            "total_fees": model["total_fees"],
            "deployed_blocks": model["deployed_blocks"]
        }

# ====================================================================
# Helper Function
# ====================================================================

def create_model(model_type: ModelType, **kwargs) -> AxionAIModel:
    """
    Factory function to create models
    
    Example:
        model = create_model(ModelType.NEURAL_NETWORK)
    """
    if model_type == ModelType.NEURAL_NETWORK:
        return NeuralNetworkModel()
    elif model_type == ModelType.RANDOM_FOREST:
        return RandomForestModel(is_classifier=kwargs.get('is_classifier', True))
    elif model_type == ModelType.GRADIENT_BOOSTING:
        return GradientBoostingModel()
    else:
        raise ValueError(f"Unsupported model type: {model_type}")
