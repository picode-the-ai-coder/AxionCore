"""
Axion Smart Contract - Python Base Class

All Python smart contracts should import from this module.
This provides access to blockchain state, events, and AXC transfers.

Installation:
    pip install axion-sdk

Usage:
    from axion_contract import SmartContract
    
    class MyContract(SmartContract):
        def initialize(self):
            self.set_state("owner", msg.sender)
        
        def my_function(self, param):
            # Your logic here
            pass
"""

from typing import Dict, List, Any, Optional
import json
import hashlib
from dataclasses import dataclass
from datetime import datetime

# ====================================================================
# Message Context (msg.*)
# ====================================================================

@dataclass
class MessageContext:
    """Context of current contract call"""
    sender: str           # Caller address
    value: int           # AXC amount sent
    gas_limit: int      # Maximum gas allowed
    block_number: int   # Current block
    timestamp: int      # Block timestamp

msg: MessageContext = None  # Injected by VM

# ====================================================================
# Event Definition
# ====================================================================

@dataclass
class Event:
    """Blockchain event"""
    event_type: str
    data: Dict[str, Any]
    block_number: int
    timestamp: int

# ====================================================================
# Smart Contract Base Class
# ====================================================================

class SmartContract:
    """
    Base class for all Axion Python smart contracts
    
    Features:
    - State persistence
    - Event emission
    - AXC transfers
    - Contract calls
    """
    
    def __init__(self):
        self.state: Dict[str, str] = {}
        self.events: List[Event] = []
        self._contract_address: str = None
        self._balance: int = 0
    
    # ================================================================
    # STATE MANAGEMENT
    # ================================================================
    
    def set_state(self, key: str, value: Any) -> None:
        """
        Persist data to blockchain
        
        Args:
            key: State key
            value: State value (will be JSON serialized)
        
        Example:
            self.set_state("owner", "0x123abc")
            self.set_state("balance", 1000)
        """
        if isinstance(value, (dict, list)):
            self.state[key] = json.dumps(value)
        else:
            self.state[key] = str(value)
    
    def get_state(self, key: str, default: Any = None) -> Any:
        """
        Retrieve persisted data from blockchain
        
        Args:
            key: State key
            default: Default value if not found
        
        Returns:
            State value
        
        Example:
            owner = self.get_state("owner")
            balance = int(self.get_state("balance", 0))
        """
        value = self.state.get(key, default)
        if value is None:
            return default
        
        # Try to parse as JSON
        try:
            return json.loads(value)
        except (json.JSONDecodeError, TypeError):
            return value
    
    def get_state_keys(self) -> List[str]:
        """Get all state keys"""
        return list(self.state.keys())
    
    def delete_state(self, key: str) -> bool:
        """Delete state entry"""
        if key in self.state:
            del self.state[key]
            return True
        return False
    
    # ================================================================
    # EVENTS
    # ================================================================
    
    def emit_event(self, event_type: str, data: Dict[str, Any] = None) -> None:
        """
        Emit a blockchain event
        
        Args:
            event_type: Type of event
            data: Event data
        
        Example:
            self.emit_event("Transfer", {
                "from": msg.sender,
                "to": recipient,
                "amount": 100
            })
        """
        if data is None:
            data = {}
        
        # Ensure all values are serializable
        serializable_data = {}
        for k, v in data.items():
            if isinstance(v, (dict, list)):
                serializable_data[k] = json.dumps(v)
            else:
                serializable_data[k] = str(v)
        
        event = Event(
            event_type=event_type,
            data=serializable_data,
            block_number=self.block_number(),
            timestamp=self.block_timestamp()
        )
        self.events.append(event)
    
    def get_events(self) -> List[Event]:
        """Get all events emitted by this contract"""
        return self.events
    
    # ================================================================
    # TRANSFERS & PAYMENTS
    # ================================================================
    
    def transfer(self, to_address: str, amount: int) -> bool:
        """
        Transfer AXC to another address
        
        Args:
            to_address: Recipient address
            amount: AXC amount (in smallest unit)
        
        Returns:
            True if successful
        
        Example:
            if self.transfer("0x456def", 100):
                self.emit_event("Transferred", {
                    "to": "0x456def",
                    "amount": 100
                })
        """
        if self._balance < amount:
            return False
        
        self._balance -= amount
        # VM will handle actual transfer
        return True
    
    def get_balance(self) -> int:
        """Get contract balance in AXC"""
        return self._balance
    
    # ================================================================
    # BLOCKCHAIN CONTEXT
    # ================================================================
    
    def block_number(self) -> int:
        """Get current block number"""
        return msg.block_number if msg else 0
    
    def block_timestamp(self) -> int:
        """Get current block timestamp"""
        return msg.timestamp if msg else 0
    
    def msg_sender(self) -> str:
        """Get address of function caller"""
        return msg.sender if msg else None
    
    def msg_value(self) -> int:
        """Get AXC amount sent with this call"""
        return msg.value if msg else 0
    
    def address(self) -> str:
        """Get this contract's address"""
        return self._contract_address
    
    # ================================================================
    # CONTRACT INTERACTION
    # ================================================================
    
    def call_contract(self,
                     contract_address: str,
                     function_name: str,
                     args: Dict[str, Any] = None) -> Any:
        """
        Call another smart contract
        
        Args:
            contract_address: Address of contract to call
            function_name: Function name
            args: Function arguments
        
        Returns:
            Return value from called function
        
        Example:
            result = self.call_contract(
                "0x789ghi",
                "transfer",
                {"to": msg.sender, "amount": 50}
            )
        """
        if args is None:
            args = {}
        
        # VM will handle actual contract call
        # This is a placeholder
        return None
    
    def static_call(self,
                   contract_address: str,
                   function_name: str,
                   args: Dict[str, Any] = None) -> Any:
        """
        Static call to another contract (read-only)
        
        Args:
            contract_address: Address of contract to call
            function_name: Function name
            args: Function arguments
        
        Returns:
            Return value from called function
        """
        if args is None:
            args = {}
        
        return None
    
    # ================================================================
    # UTILITIES
    # ================================================================
    
    def require(self, condition: bool, message: str = "Requirement failed") -> None:
        """
        Check a condition, revert if false
        
        Args:
            condition: Condition to check
            message: Error message
        
        Example:
            self.require(msg.sender == owner, "Not owner")
        """
        if not condition:
            raise AssertionError(message)
    
    def assert_(self, condition: bool, message: str = "Assertion failed") -> None:
        """Assert a condition"""
        if not condition:
            raise AssertionError(message)
    
    def revert(self, reason: str = "Reverted") -> None:
        """Revert contract execution"""
        raise RuntimeError(reason)
    
    def log(self, message: str) -> None:
        """Log message for debugging"""
        timestamp = datetime.now().isoformat()
        print(f"[{timestamp}] {message}")
    
    # ================================================================
    # CRYPTOGRAPHY
    # ================================================================
    
    def sha256(self, data: str) -> str:
        """
        SHA256 hash
        
        Example:
            hash_value = self.sha256("hello")
        """
        return hashlib.sha256(data.encode()).hexdigest()
    
    def keccak256(self, data: str) -> str:
        """
        Keccak256 hash (like Ethereum)
        
        Note: Requires PyKeccak library
        """
        try:
            from Crypto.Hash import keccak
            k = keccak.new(digest_bits=256)
            k.update(data.encode())
            return k.hexdigest()
        except ImportError:
            raise ImportError("Install pycryptodome: pip install pycryptodome")
    
    def recover_address(self, message: str, signature: str) -> Optional[str]:
        """
        Recover address from signed message
        
        Example:
            signer = self.recover_address(message, signature)
        """
        # VM will handle signature recovery
        return None
