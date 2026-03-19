# 🚀 PHASE 2 - SMART CONTRACTS & AXIONAI

**Python & JavaScript Smart Contracts + AI Integration**

---

## 📚 Table of Contents

1. [Quick Start](#quick-start)
2. [Python Smart Contracts](#python-smart-contracts)
3. [JavaScript Smart Contracts](#javascript-smart-contracts)
4. [AxionAI Integration](#axionai-integration)
5. [Examples](#examples)
6. [API Reference](#api-reference)
7. [Security](#security)

---

## 🚀 Quick Start

### Install SDK

```bash
# Python SDK
pip install axion-sdk numpy scikit-learn tensorflow

# JavaScript SDK
npm install @axionjs/contracts
```

### Your First Python Contract

```python
# contracts/my_contract.py
from axion_contract import SmartContract

class MyContract(SmartContract):
    def initialize(self):
        self.set_state("counter", 0)
    
    def increment(self):
        count = int(self.get_state("counter", 0))
        self.set_state("counter", count + 1)
        self.emit_event("Incremented", {"count": count + 1})
        return {"count": count + 1}
```

### Deploy

```bash
# Deploy contract
axion_node deploy contracts/my_contract.py \
    --runtime python \
    --gas-limit 1000000

# Call function
axion_node call <contract_address> increment
```

---

## 🐍 Python Smart Contracts

### Contract Structure

All Python contracts inherit from `SmartContract`:

```python
from axion_contract import SmartContract

class MyContract(SmartContract):
    def __init__(self):
        super().__init__()
        # Initialize state
    
    def my_function(self):
        # Your logic
        pass
```

### State Management

```python
# Set state (persisted to blockchain)
self.set_state("owner", msg.sender)
self.set_state("balance", 1000)

# Get state
owner = self.get_state("owner")
balance = int(self.get_state("balance", 0))

# Delete state
self.delete_state("temp_data")
```

### Events

```python
# Emit event
self.emit_event("Transfer", {
    "from": msg.sender,
    "to": recipient,
    "amount": amount
})

# Get all events
events = self.get_events()
```

### Transfers

```python
# Send AXC
if self.transfer(recipient, 100):
    print("Transfer successful")

# Get balance
balance = self.get_balance()
```

### Message Context

```python
# Current caller
sender = msg.sender

# AXC sent with call
amount = msg.value

# Blockchain info
block = self.block_number()
time = self.block_timestamp()
```

### Contract Interaction

```python
# Call another contract
result = self.call_contract(
    "0xabc123",
    "transfer",
    {"to": recipient, "amount": 100}
)

# Static call (read-only)
balance = self.static_call(
    "0xabc123",
    "balance_of",
    {"account": msg.sender}
)
```

### Error Handling

```python
# Require condition (revert if false)
self.require(msg.sender == owner, "Only owner")

# Revert execution
if error_condition:
    self.revert("Something went wrong")

# Assert
self.assert_(amount > 0, "Amount must be positive")
```

---

## 🔗 JavaScript Smart Contracts

### Contract Structure

```javascript
class MyContract extends SmartContract {
    constructor() {
        super();
        // Initialize
    }
    
    myFunction() {
        // Your logic
    }
}
```

### State Management

```javascript
// Set state
this.setState("owner", msg.sender);
this.setState("balance", 1000);

// Get state
let owner = this.getState("owner");
let balance = parseInt(this.getState("balance", 0));

// Delete state
this.deleteState("tempData");
```

### Events

```javascript
// Emit event
this.emitEvent("Transfer", {
    from: msg.sender,
    to: recipient,
    amount: amount
});
```

### Transfers

```javascript
// Send AXC
if (this.transfer(recipient, 100)) {
    console.log("Transfer successful");
}

// Get balance
let balance = this.getBalance();
```

### Contract Calls

```javascript
// Call another contract
let result = this.callContract(
    "0xabc123",
    "transfer",
    {to: recipient, amount: 100}
);

// Static call
let balance = this.staticCall(
    "0xabc123",
    "balanceOf",
    {account: msg.sender}
);
```

### Error Handling

```javascript
// Require condition
this.require(msg.sender === owner, "Only owner");

// Revert
if (errorCondition) {
    this.revert("Something went wrong");
}
```

---

## 🤖 AxionAI Integration

### Create & Train Models

```python
from axion_ai_v2 import create_model, ModelType

# Create neural network
model = create_model(ModelType.NEURAL_NETWORK)

# Train model
model.train(X_train, y_train, epochs=100)

# Validate
metrics = model.validate(X_test, y_test)
print(metrics)
```

### Deploy to Blockchain

```python
# Deploy trained model
result = model.deploy_to_blockchain()

contract_address = result['contract_address']
model_hash = result['model_hash']
print(f"Model deployed at: {contract_address}")
```

### Run On-Chain Inference

```python
# Run inference
prediction = model.run_inference_on_chain(
    contract_address,
    X_new
)

print(prediction['prediction'])
```

### Model Types Supported

```python
ModelType.LINEAR_REGRESSION      # Linear regression
ModelType.LOGISTIC_REGRESSION    # Logistic regression
ModelType.RANDOM_FOREST          # Random forest
ModelType.NEURAL_NETWORK         # Deep learning
ModelType.SVM                    # Support vector machines
ModelType.GRADIENT_BOOSTING      # XGBoost/LightGBM
ModelType.CLUSTERING             # K-means, DBSCAN
```

### Train Custom Model

```python
from axion_ai_v2 import NeuralNetworkModel

# Create custom neural network
model = NeuralNetworkModel(name="MyModel")

# Train with custom parameters
model.train(
    X_train, y_train,
    epochs=200,
    batch_size=16
)

# Deploy
deployment = model.deploy_to_blockchain()
```

---

## 📋 Examples

### Example 1: Simple Counter

**Python:**
```python
from axion_contract import SmartContract

class Counter(SmartContract):
    def initialize(self):
        self.set_state("count", "0")
    
    def increment(self):
        count = int(self.get_state("count"))
        self.set_state("count", str(count + 1))
        self.emit_event("Incremented", {"count": count + 1})
        return {"count": count + 1}
    
    def get_count(self):
        return {"count": int(self.get_state("count"))}
```

**JavaScript:**
```javascript
class Counter extends SmartContract {
    initialize() {
        this.setState("count", "0");
    }
    
    increment() {
        let count = parseInt(this.getState("count"));
        this.setState("count", (count + 1).toString());
        this.emitEvent("Incremented", {count: count + 1});
        return {count: count + 1};
    }
    
    getCount() {
        return {count: parseInt(this.getState("count"))};
    }
}
```

### Example 2: ERC20 Token

See: `contracts/python/examples/erc20_token.py`

Features:
- Token transfers
- Allowances
- Minting and burning
- Full ERC20 compatibility

### Example 3: AI Price Prediction

```python
from axion_contract import SmartContract
from axion_ai_v2 import create_model, ModelType
import numpy as np

class PricePredictionOracle(SmartContract):
    def initialize(self, model_address):
        self.set_state("model_address", model_address)
        self.set_state("predictions", "0")
    
    def predict_price(self, features_json):
        """Predict price using AI model"""
        import json
        features = json.loads(features_json)
        X = np.array(features).reshape(1, -1)
        
        # Call AI model
        model_addr = self.get_state("model_address")
        result = self.call_contract(
            model_addr,
            "predict",
            {"input": features_json}
        )
        
        # Store prediction
        self.emit_event("PredictionMade", {
            "prediction": result,
            "timestamp": self.block_timestamp()
        })
        
        return {"prediction": result}
```

---

## 📖 API Reference

### SmartContract Base Class

#### State Methods
```python
set_state(key: str, value: Any) -> None
get_state(key: str, default: Any = None) -> Any
delete_state(key: str) -> bool
get_state_keys() -> List[str]
```

#### Event Methods
```python
emit_event(event_type: str, data: Dict[str, Any] = None) -> None
get_events() -> List[Event]
```

#### Transfer Methods
```python
transfer(to_address: str, amount: int) -> bool
get_balance() -> int
```

#### Context Methods
```python
block_number() -> int
block_timestamp() -> int
msg_sender() -> str
msg_value() -> int
address() -> str
```

#### Contract Methods
```python
call_contract(contract_address: str, function_name: str, args: Dict) -> Any
static_call(contract_address: str, function_name: str, args: Dict) -> Any
```

#### Utility Methods
```python
require(condition: bool, message: str) -> None
revert(reason: str) -> None
log(message: str) -> None
```

#### Crypto Methods
```python
sha256(data: str) -> str
keccak256(data: str) -> str
recover_address(message: str, signature: str) -> Optional[str]
```

---

## 🔐 Security

### Best Practices

1. **Check Inputs**
   ```python
   self.require(amount > 0, "Amount must be positive")
   self.require(to_address is not None, "Invalid address")
   ```

2. **Verify Caller**
   ```python
   owner = self.get_state("owner")
   self.require(msg.sender == owner, "Only owner")
   ```

3. **State Consistency**
   ```python
   # Update state atomically
   self.set_state("balance", new_balance)
   self.set_state("last_update", self.block_number())
   ```

4. **Overflow Protection**
   ```python
   # Python handles big integers, but check limits
   self.require(amount <= MAX_AMOUNT, "Amount too large")
   ```

### Gas Limits

Each contract has a gas limit:
- Default: 1,000,000 gas
- Can be increased during deployment
- Exceeding limit causes revert

### Sandboxing

- Python contracts run in restricted sandbox
- No network access
- No file system access
- Limited imports

---

## 📦 File Structure

```
contracts/
├── python/
│   ├── axion_contract.py          # Base class
│   ├── examples/
│   │   ├── erc20_token.py
│   │   ├── auction.py
│   │   └── voting.py
│   └── tests/
│       └── test_contracts.py
├── javascript/
│   ├── ContractBase.js
│   ├── examples/
│   │   ├── ERC20Token.js
│   │   └── Auction.js
│   └── tests/
│       └── contracts.test.js
└── README.md

axion_ai_v2.py              # AI integration
avm_v2.h                    # Enhanced VM
contract_apis.h             # API definitions
```

---

## 🔄 Workflow

### Develop
```bash
# Write contract
code contracts/my_contract.py

# Test locally
python -m pytest contracts/python/tests/
```

### Deploy
```bash
# Deploy to blockchain
axion_node deploy contracts/my_contract.py

# Get contract address
export CONTRACT=0xabc123
```

### Interact
```bash
# Call function
axion_node call $CONTRACT my_function --args '{"param": "value"}'

# Get state
axion_node state $CONTRACT key_name
```

### Monitor
```bash
# Get events
axion_node events $CONTRACT

# Get statistics
axion_node stats $CONTRACT
```

---

## 🎓 Learning Path

1. **Beginner**: Counter contract
2. **Intermediate**: ERC20 token
3. **Advanced**: DAO voting contract
4. **Expert**: AI oracle contract

---

## 🤝 Contributing

Submit smart contracts:
1. Test thoroughly
2. Document API
3. Include examples
4. Add security notes

---

## 📞 Support

- **Documentation**: See guides in repository
- **Discord**: Community support channel
- **GitHub**: Issue tracker
- **Email**: support@axiondigitaverse.com

---

*Ready to build decentralized applications on Axion!* 🚀
